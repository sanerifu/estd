#ifndef __ESTD_STRING_BUILDER_H__
#define __ESTD_STRING_BUILDER_H__

#include <stddef.h>

#include "arena.h"
#include "result.h"
#include "str.h"

typedef struct EstdStringBuilder EstdStringBuilder;
struct EstdStringBuilder {
    EstdStringBuilder* prev;
    size_t length;
    size_t total_length;
    char data[];
};

extern EstdResult estd_string_builder_append(EstdStringBuilder** io_self, EstdString string, EstdArena** allocator);
extern EstdResult estd_string_builder_appendf(EstdStringBuilder** io_self, EstdArena** allocator, char const* fmt, ...);
extern EstdResult
estd_string_builder_build(EstdString* o_ret, EstdStringBuilder const* const* i_self, EstdArena** allocator);
extern size_t estd_string_builder_length(EstdStringBuilder* i_self);

#endif

#if (!defined(ESTD_STRING_BUILDER_IMPLEMENTATION) || defined(ESTD_ALL_IMPLEMENTATION)) && \
    !defined(__ESTD_STRING_BUILDER_C__)
#define __ESTD_STRING_BUILDER_C__

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EstdResult estd_string_builder_append(EstdStringBuilder** io_self, EstdString string, EstdArena** allocator) {
    EstdStringBuilder* self = *io_self;
    size_t previous_total_length = self == NULL ? 0 : self->total_length;
    EstdStringBuilder* next = NULL;
    ESTD_BUBBLE(
        estd_arena_fsm(&next, allocator, string.length),
        "Could not append string \"%" PRIestr "\"",
        ESTD_STRING_ARG(string)
    );

    next->prev = self;
    next->length = string.length;
    next->total_length = previous_total_length + next->length;
    memcpy(next->data, string.data, string.length);

    self = next;
    *io_self = self;
    return ESTD_SUCCESS;
}

EstdResult estd_string_builder_appendf(EstdStringBuilder** io_self, EstdArena** allocator, char const* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t length = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    va_start(ap, fmt);
    EstdStringBuilder* self = *io_self;
    size_t previous_total_length = self == NULL ? 0 : self->total_length;
    EstdStringBuilder* next = NULL;
    ESTD_BUBBLE(estd_arena_fsm(&next, allocator, length + 1), "Could not append format");

    next->prev = self;
    next->length = length;
    next->total_length = previous_total_length + next->length;
    vsnprintf(next->data, length + 1, fmt, ap);
    va_end(ap);

    self = next;
    *io_self = self;
    return ESTD_SUCCESS;
}

EstdResult estd_string_builder_build(EstdString* o_ret, EstdStringBuilder const* const* i_self, EstdArena** allocator) {
    EstdStringBuilder const* self = *i_self;
    if (self == NULL) {
        *o_ret = ESTD_STRING(NULL, 0);
        return ESTD_SUCCESS;
    }
    EstdString ret = ESTD_STRING(NULL, self->total_length);
    ESTD_BUBBLE(
        estd_arena_array(&ret.data, allocator, ret.length + 1),
        "Could not build string of length %zu",
        ret.length
    );
    ret.data[ret.length] = '\0';

    size_t offset = ret.length;
    while (self != NULL) {
        memcpy(ret.data + offset - self->length, self->data, self->length);
        offset -= self->length;
        EstdStringBuilder* prev = self->prev;
        self = prev;
    }

    *o_ret = ret;
    return ESTD_SUCCESS;
}

size_t estd_string_builder_length(EstdStringBuilder* i_self) {
    return i_self == NULL ? 0 : i_self->total_length;
}

EstdResult estd_read_stream(EstdString* o_ret, EstdArena** allocator, FILE* fp) {
    EstdStringBuilder* builder = NULL;
    EstdArena* ESTD_CLEAN(estd_arena_destroy) temp = NULL;
    char buf[BUFSIZ];
    size_t size = 0;
    while ((size = fread(buf, sizeof(char), sizeof(buf), fp))) {
        estd_string_builder_append(&builder, ESTD_STRING(buf, size), &temp);
        if (size < sizeof(buf)) {
            if (ferror(fp)) {
                ESTD_THROW(ESTD_IO_ERROR, "Could not read stream: %s", strerror(errno));
            }
            break;
        }
    }
    ESTD_BUBBLE(estd_string_builder_build(o_ret, &builder, allocator), "Could not build the read stream");
    return ESTD_SUCCESS;
}

#endif
