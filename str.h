#ifndef __ESTD_STRING_H__
#define __ESTD_STRING_H__

#include <stddef.h>
#include <string.h>

#include "arena.h"
#include "result.h"

typedef struct EstdString EstdString;
struct EstdString {
    size_t length;
    char* data;
};

#define ESTD_STRING(_data, _length) ((EstdString){.length = (_length), .data = (_data)})
#define ESTD_LITERAL(literal) ESTD_STRING((char*)(literal), sizeof((literal)) - 1)
#define ESTD_CTRING(cstr) ESTD_STRING((cstr), strlen((cstr)))
#define ESTD_CHAR(c) ESTD_STRING(((char[]){(c)}), 1)
#define ESTD_STRING_ARG(s) (int)((s).length), (s).data
#define PRIestr ".*s"

extern EstdString estd_string_split(EstdString* io_string, EstdString delimiter);
extern EstdString estd_string_trim(EstdString string);
extern int estd_string_compare(EstdString left, EstdString right);
extern EstdResult estd_string_duplicate(EstdString* o_ret, EstdString string, EstdArena** allocator);
extern EstdResult estd_string_format(EstdString* o_ret, EstdArena** allocator, char const* fmt, ...);
extern EstdResult estd_read_file(EstdString* o_ret, EstdArena** allocator, FILE* fp);
extern EstdResult estd_string_url_decode(EstdString* o_ret, EstdString string, EstdArena** allocator);
extern EstdResult estd_string_url_encode(EstdString* o_ret, EstdString string, EstdArena** allocator);

#endif

#if (!defined(ESTD_STRING_IMPLEMENTATION) || defined(ESTD_ALL_IMPLEMENTATION)) && !defined(__ESTD_STRING_C__)
#define __ESTD_STRING_C__

#include <ctype.h>
#include <iso646.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

EstdString estd_string_split(EstdString* io_string, EstdString delimiter) {
    EstdString string = *io_string;
    if (string.length < delimiter.length) {
        return ESTD_STRING(NULL, 0);
    }
    EstdString ret = ESTD_STRING(string.data, 0);
    bool found = false;

    while ((string.length - ret.length) >= delimiter.length) {
        if (memcmp(string.data + ret.length, delimiter.data, delimiter.length) == 0) {
            found = true;
            break;
        }
        ret.length += 1;
    }

    if (found) {
        string.length -= ret.length + delimiter.length;
        string.data += ret.length + delimiter.length;
    } else {
        ret.length = string.length;
        string.length = 0;
    }

    *io_string = string;
    return ret;
}

EstdString estd_string_trim(EstdString string) {
    if (string.length == 0) {
        return string;
    }
    size_t start = 0;
    size_t end = string.length - 1;
    while (start < string.length and isblank(string.data[start])) {
        start += 1;
    }
    if (start == string.length) {
        return ESTD_LITERAL("");
    }
    while (end > 0 and isblank(string.data[end])) {
        end -= 1;
    }

    return ESTD_STRING(string.data + start, end - start + 1);
}

int estd_string_compare(EstdString left, EstdString right) {
    size_t min_length = left.length < right.length ? left.length : right.length;
    int result = memcmp(left.data, right.data, min_length);
    if (result == 0) {
        return (int)left.length - (int)right.length;
    } else {
        return result;
    }
}

EstdResult estd_string_duplicate(EstdString* o_ret, EstdString string, EstdArena** allocator) {
    EstdString ret;
    ESTD_BUBBLE(estd_arena_array(&ret.data, allocator, string.length + 1), "Could not duplicate string");
    memcpy(ret.data, string.data, string.length);
    ret.length = string.length;
    ret.data[ret.length] = '\0';
    *o_ret = ret;
    return ESTD_SUCCESS;
}

EstdResult estd_string_format(EstdString* o_ret, EstdArena** allocator, char const* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t length = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    EstdString ret;
    ESTD_BUBBLE(estd_arena_array(&ret.data, allocator, length + 1), "Could not allocate formatted string");

    va_start(ap, fmt);
    ret.length = length;
    vsnprintf(ret.data, ret.length + 1, fmt, ap);
    ret.data[ret.length] = '\0';
    va_end(ap);

    *o_ret = ret;
    return ESTD_SUCCESS;
}

EstdResult estd_read_file(EstdString* o_ret, EstdArena** allocator, FILE* fp) {
    EstdString ret;

    fseek(fp, 0, SEEK_END);
    ret.length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    ESTD_BUBBLE(estd_arena_array(&ret.data, allocator, ret.length + 1), "Could not create buffer to read file");

    fread(ret.data, sizeof(char), ret.length, fp);
    ret.data[ret.length] = '\0';

    *o_ret = ret;
    return ESTD_SUCCESS;
}

static uint8_t hex2nibble(char digit) {
    if ('0' <= digit and digit <= '9') {
        return digit - '0';
    } else if ('A' <= digit and digit <= 'F') {
        return digit - 'A' + 10;
    } else {
        return digit - 'a' + 10;
    }
}

static uint8_t hex2byte(char upper, char lower) {
    return (hex2nibble(upper) << 4) | (hex2nibble(lower));
}

EstdResult estd_string_url_decode(EstdString* o_ret, EstdString string, EstdArena** allocator) {
    EstdString ret;
    ESTD_BUBBLE(estd_arena_array(&ret.data, allocator, string.length), "Could not allocate decoded string");
    ret.length = 0;

    for (size_t i = 0; i < string.length; i++) {
        if (string.data[i] == '%') {
            if (i > string.length - 3 || !isxdigit(string.data[i + 1]) || !isxdigit(string.data[i + 2])) {
                ESTD_THROW(ESTD_INVALID_PERCENT_ENCODING, "\"%" PRIestr "\" at index %zu", ESTD_STRING_ARG(string), i);
            }
            char c = (char)hex2byte(string.data[i + 1], string.data[i + 2]);
            ret.data[ret.length] = c;
            ret.length += 1;
            i += 2;
        } else if (string.data[i] == '+') {
            ret.data[ret.length] = ' ';
            ret.length += 1;
        } else {
            ret.data[ret.length] = string.data[i];
            ret.length += 1;
        }
    }

    *o_ret = ret;
    return ESTD_SUCCESS;
}

static char nibble2hex(uint8_t nibble) {
    if (0x0 <= nibble and nibble <= 0x9) {
        return nibble + '0';
    } else if (0xA <= nibble and nibble <= 0xF) {
        return nibble + 'A' - 10;
    } else {
        return '0';
    }
}

static uint16_t byte2hex(uint8_t byte) {
    uint16_t upper = (uint16_t)nibble2hex((byte & 0xF0) >> 4);
    uint16_t lower = (uint16_t)nibble2hex(byte & 0xF);
    return (upper << 8) | lower;
}

EstdResult estd_string_url_encode(EstdString* o_ret, EstdString string, EstdArena** allocator) {
    EstdString ret;

    EstdString temp;
    EstdArena* ESTD_CLEAN(estd_arena_destroy) temp_allocator = {0};
    ESTD_BUBBLE(
        estd_arena_array(&temp.data, &temp_allocator, string.length * 3),
        "Could not allocate temporary encoded string"
    );
    temp.length = 0;

    for (size_t i = 0; i < string.length; i++) {
        if (isalnum(string.data[i])) {
            temp.data[temp.length] = string.data[i];
            temp.length += 1;
        } else if (string.data[i] == ' ') {
            temp.data[temp.length] = '+';
            temp.length += 1;
        } else {
            uint16_t hexes = byte2hex(string.data[i]);
            char first = (hexes & 0xFF00) >> 8;
            char second = hexes & 0xFF;
            temp.data[temp.length] = '%';
            temp.data[temp.length + 1] = first;
            temp.data[temp.length + 2] = second;
            temp.length += 3;
        }
    }

    ESTD_BUBBLE(estd_arena_array(&ret.data, allocator, temp.length + 1), "Could not allocate encoded string");
    ret.length = temp.length;
    memcpy(ret.data, temp.data, temp.length * sizeof(char));
    ret.data[ret.length] = '\0';

    *o_ret = ret;

    return ESTD_SUCCESS;
}

#endif