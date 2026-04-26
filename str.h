#ifndef __ESTD_STRING_H__
#define __ESTD_STRING_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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
extern void estd_string_tolower(EstdString mut_self);
extern void estd_string_toupper(EstdString mut_self);
extern bool estd_string_has_prefix(EstdString self, EstdString prefix);
extern bool estd_string_has_suffix(EstdString self, EstdString suffix);
extern void estd_string_reverse(EstdString mut_self);
extern EstdResult estd_string_to_int(intmax_t* o_ret, EstdString self, int base);
extern EstdResult estd_string_to_uint(uintmax_t* o_ret, EstdString self, int base);
extern int estd_string_scan(EstdString self, char const* fmt, ...);

#endif

#if (defined(ESTD_STRING_IMPLEMENTATION) || defined(ESTD_ALL_IMPLEMENTATION)) && !defined(__ESTD_STRING_C__)
#define __ESTD_STRING_C__

#include <ctype.h>
#include <inttypes.h>
#include <iso646.h>
#include <stdarg.h>
#include <stdlib.h>

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
    while (start < string.length and isspace(string.data[start])) {
        start += 1;
    }
    if (start == string.length) {
        return ESTD_LITERAL("");
    }
    while (end > 0 and isspace(string.data[end])) {
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

void estd_string_tolower(EstdString mut_self) {
    for (size_t i = 0; i < mut_self.length; i++) {
        mut_self.data[i] = tolower(mut_self.data[i]);
    }
}

void estd_string_toupper(EstdString mut_self) {
    for (size_t i = 0; i < mut_self.length; i++) {
        mut_self.data[i] = toupper(mut_self.data[i]);
    }
}

bool estd_string_has_prefix(EstdString self, EstdString prefix) {
    if (self.length < prefix.length) {
        return false;
    }
    for (size_t i = 0; i < prefix.length; i++) {
        if (self.data[i] != prefix.data[i]) {
            return false;
        }
    }
    return true;
}

bool estd_string_has_suffix(EstdString self, EstdString suffix) {
    if (self.length < suffix.length) {
        return false;
    }
    size_t diff = self.length - suffix.length;
    for (size_t i = 0; i < suffix.length; i++) {
        if (self.data[i + diff] != suffix.data[i]) {
            return false;
        }
    }
    return true;
}

void estd_string_reverse(EstdString mut_self) {
    for (size_t i = 0; i < mut_self.length / 2; i++) {
        char temp = mut_self.data[i];
        mut_self.data[i] = mut_self.data[mut_self.length - 1 - i];
        mut_self.data[mut_self.length - 1 - i] = temp;
    }
}

EstdResult estd_string_to_int(intmax_t* o_ret, EstdString self, int base) {
    intmax_t ret = 0;
    int sign = 1;
    while (self.length != 0 && isspace(self.data[0])) {
        self.data += 1;
        self.length -= 1;
    }
    if (self.length == 0) {
        ESTD_THROW(ESTD_ILLEGAL_NUMBER, "%" PRIestr, ESTD_STRING_ARG(self));
    }
    if (self.length >= 2 && self.data[0] == '-') {
        sign = -1;
        self.length -= 1;
        self.data += 1;
    }
    while (self.length != 0) {
        int digit = 0;
        if ('0' <= self.data[0] && self.data[0] <= '9') {
            digit = self.data[0] - '0';
        } else if ('a' <= self.data[0] && self.data[0] <= 'z') {
            digit = self.data[0] - 'a' + 10;
        } else if ('A' <= self.data[0] && self.data[0] <= 'Z') {
            digit = self.data[0] - 'A' + 10;
        } else {
            ESTD_THROW(ESTD_ILLEGAL_NUMBER, "%" PRIestr, ESTD_STRING_ARG(self));
        }
        if (digit >= base) {
            ESTD_THROW(ESTD_ILLEGAL_NUMBER, "%" PRIestr, ESTD_STRING_ARG(self));
        }
        if (ret > (INTMAX_MAX - digit) / base) {
            ESTD_THROW(ESTD_OVERFLOW, "%" PRIestr, ESTD_STRING_ARG(self));
        }
        ret = base * ret + digit;
        self.length -= 1;
        self.data += 1;
    }
    *o_ret = sign * ret;
    return ESTD_SUCCESS;
}

EstdResult estd_string_to_uint(uintmax_t* o_ret, EstdString self, int base) {
    uintmax_t ret = 0;
    while (self.length != 0 && isspace(self.data[0])) {
        self.data += 1;
        self.length -= 1;
    }
    if (self.length == 0) {
        ESTD_THROW(ESTD_ILLEGAL_NUMBER, "%" PRIestr, ESTD_STRING_ARG(self));
    }
    while (self.length != 0) {
        int digit = 0;
        if ('0' <= self.data[0] && self.data[0] <= '9') {
            digit = self.data[0] - '0';
        } else if ('a' <= self.data[0] && self.data[0] <= 'z') {
            digit = self.data[0] - 'a' + 10;
        } else if ('A' <= self.data[0] && self.data[0] <= 'Z') {
            digit = self.data[0] - 'A' + 10;
        } else {
            ESTD_THROW(ESTD_ILLEGAL_NUMBER, "%" PRIestr, ESTD_STRING_ARG(self));
        }
        if (digit >= base) {
            ESTD_THROW(ESTD_ILLEGAL_NUMBER, "%" PRIestr, ESTD_STRING_ARG(self));
        }
        if (ret > (UINTMAX_MAX - digit) / base) {
            ESTD_THROW(ESTD_OVERFLOW, "%" PRIestr, ESTD_STRING_ARG(self));
        }
        ret = base * ret + digit;
        self.length -= 1;
        self.data += 1;
    }
    *o_ret = ret;
    return ESTD_SUCCESS;
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

int estd_string_scan(EstdString self, char const* fmt, ...) {
    char* copy = (char*)calloc(self.length + 1, sizeof(char));
    memcpy(copy, self.data, self.length);
    va_list ap;
    va_start(ap, fmt);
    int result = vsscanf(copy, fmt, ap);
    va_end(ap);
    free(copy);
    return result;
}

#endif