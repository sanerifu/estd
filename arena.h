#ifndef __ESTD_ARENA_H__
#define __ESTD_ARENA_H__

#include <stddef.h>

#include "result.h"

typedef struct EstdArena EstdArena;
struct EstdArena {
    EstdArena* prev;
    size_t size;
    size_t capacity;
    char data[];
};

extern EstdResult estd_arena_allocate(void** o_ret, EstdArena** io_self, size_t size, size_t alignment);
#define estd_arena_new(o_ret, io_self) estd_arena_allocate((void**)o_ret, io_self, sizeof(**o_ret), alignof(**o_ret))
#define estd_arena_array(o_ret, io_self, length) \
    estd_arena_allocate((void**)o_ret, io_self, sizeof(**o_ret) * (length), alignof(**o_ret))
#define estd_arena_fsm(o_ret, io_self, size) \
    estd_arena_allocate((void**)o_ret, io_self, sizeof(**o_ret) + size, alignof(**o_ret))
extern void estd_arena_destroy(EstdArena** io_self);
extern void estd_arena_destroyWrapper(void* data);

#ifndef ESTD_CLEAN
#define ESTD_CLEAN(f) __attribute__((cleanup(f##Wrapper)))
#endif

#endif

#if (defined(ESTD_ARENA_IMPLEMENTATION) || defined(ESTD_ALL_IMPLEMENTATION)) && !defined(__ESTD_ARENA_C__)
#define __ESTD_ARENA_C__

// #undef estd_arena_allocate

#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>

static uintptr_t align(uintptr_t value, size_t alignment) {
    return (value + (alignment - 1)) & ~(alignment - 1);
}

EstdResult estd_arena_allocate(void** o_ret, EstdArena** io_self, size_t size, size_t alignment) {
    EstdArena* self = *io_self;
    if (self == NULL) {
        self = (EstdArena*)calloc(1, sizeof(EstdArena) + size + alignment);
        if (self == NULL) {
            ESTD_THROW(ESTD_OUT_OF_MEMORY, "Could not allocate the arena");
        }
        self->capacity = size + alignment;
        self->size = align((uintptr_t)self->data, alignment) - (uintptr_t)self->data;
        self->prev = NULL;

        *o_ret = self->data + self->size;
        self->size += size;
        *io_self = self;
        return ESTD_SUCCESS;
    }

    size_t padding_previous =
        align((uintptr_t)self->data + self->size, alignment) - ((uintptr_t)self->data + self->size);

    if (self->capacity - self->size < size + padding_previous) {
        size_t new_size = size > self->capacity * 2 ? size : self->capacity * 2;
        EstdArena* new_node = (EstdArena*)calloc(1, sizeof(EstdArena) + new_size + alignment);
        if (new_node == NULL) {
            ESTD_THROW(ESTD_OUT_OF_MEMORY, "Could not grow the arena");
        }
        new_node->capacity = new_size + alignment;
        new_node->size = align((uintptr_t)new_node->data, alignment) - (uintptr_t)new_node->data;
        new_node->prev = self;
        self = new_node;

        *o_ret = self->data + self->size;
        self->size += size;
        *io_self = self;
        return ESTD_SUCCESS;
    }

    *o_ret = self->data + padding_previous + self->size;
    self->size += size + padding_previous;
    *io_self = self;
    return ESTD_SUCCESS;
}

void estd_arena_destroy(EstdArena** io_self) {
    EstdArena* self = *io_self;
    while (self != NULL) {
        EstdArena* prev = self->prev;
        free(self);
        self = prev;
    }
    *io_self = NULL;
}

void estd_arena_destroyWrapper(void* data) {
    estd_arena_destroy((EstdArena**)data);
}

#endif
