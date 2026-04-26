#ifndef __ESTD_RESULT_H__
#define __ESTD_RESULT_H__

#include "log.h"

typedef enum {
    ESTD_SUCCESS,
    ESTD_OUT_OF_MEMORY,
    ESTD_INVALID_PERCENT_ENCODING,
    ESTD_ILLEGAL_NUMBER,
    ESTD_OVERFLOW,
    ESTD_IO_ERROR
} EstdResult;

#define ESTD_THROW(result, fmt, ...)                 \
    do {                                             \
        ESTD_ERROR(#result ": " fmt, ##__VA_ARGS__); \
        return result;                               \
    } while (0)

#define ESTD_BUBBLE(expr, fmt, ...)                         \
    do {                                                    \
        EstdResult ___estdmacro_result;                     \
        if ((___estdmacro_result = expr) != ESTD_SUCCESS) { \
            ESTD_TRACE(fmt, ##__VA_ARGS__);                 \
            return ___estdmacro_result;                     \
        }                                                   \
    } while (0)

#define ESTD_ASSERT(result, expr, fmt, ...)                                           \
    do {                                                                              \
        if (!(expr)) {                                                                \
            ESTD_ASSERTION("Assertion (" #expr ") " #result ": " fmt, ##__VA_ARGS__); \
            return result;                                                            \
        }                                                                             \
    } while (0)

#define ESTD_PANIC(result, expr, fmt, ...)                                            \
    do {                                                                              \
        if (!(expr)) {                                                                \
            ESTD_ASSERTION("Assertion (" #expr ") " #result ": " fmt, ##__VA_ARGS__); \
            abort();                                                                  \
        }                                                                             \
    } while (0)

#endif
