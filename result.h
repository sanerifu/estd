#ifndef __ESTD_RESULT_H__
#define __ESTD_RESULT_H__

#include <stdlib.h>

#include "log.h"

#define ___ESTD_BLANK
#define ___ESTD_MAKE_RESULT(name) extern char const name[];
#define ___ESTD_MAKE_RESULT_DATA(name) char const name[] = #name;
#define ESTD_RESULT(name, results) \
    typedef char const* name;      \
    results(___ESTD_MAKE_RESULT, ___ESTD_BLANK)
#define ESTD_RESULT_DATA(results) results(___ESTD_MAKE_RESULT_DATA, ___ESTD_BLANK)

#define ESTD_SUCCESS NULL
#define ___ESTD_RESULTS(RESULT, SEP)          \
    RESULT(ESTD_OUT_OF_MEMORY)                \
    SEP RESULT(ESTD_INVALID_PERCENT_ENCODING) \
    SEP RESULT(ESTD_ILLEGAL_NUMBER)           \
    SEP RESULT(ESTD_OVERFLOW)                 \
    SEP RESULT(ESTD_IO_ERROR)                 \
    SEP RESULT(ESTD_MISSING_ARGUMENT)         \
    SEP RESULT(ESTD_UNKNOWN_ARGUMENT)         \
    SEP RESULT(ESTD_INVALID_ENUM)

ESTD_RESULT(EstdResult, ___ESTD_RESULTS);

#define ESTD_THROW(result, fmt, ...)                 \
    do {                                             \
        ESTD_ERROR(#result ": " fmt, ##__VA_ARGS__); \
        return result;                               \
    } while (0)

#define ESTD_BUBBLE(expr, fmt, ...)                                       \
    do {                                                                  \
        EstdResult ___estdmacro_result;                                   \
        if ((___estdmacro_result = (EstdResult)(expr)) != ESTD_SUCCESS) { \
            ESTD_TRACE(fmt, ##__VA_ARGS__);                               \
            return ___estdmacro_result;                                   \
        }                                                                 \
    } while (0)

#define ESTD_BUBBLE_INT(expr, fmt, ...)                                   \
    do {                                                                  \
        EstdResult ___estdmacro_result;                                   \
        if ((___estdmacro_result = (EstdResult)(expr)) != ESTD_SUCCESS) { \
            ESTD_TRACE(fmt, ##__VA_ARGS__);                               \
            return (int)(___estdmacro_result != NULL);                    \
        }                                                                 \
    } while (0)

#define ESTD_ASSERT(result, expr, fmt, ...)                                           \
    do {                                                                              \
        if (!(expr)) {                                                                \
            ESTD_ASSERTION("Assertion (" #expr ") " #result ": " fmt, ##__VA_ARGS__); \
            return result;                                                            \
        }                                                                             \
    } while (0)

#define ESTD_ASSERT_PANIC(expr, fmt, ...)                                       \
    do {                                                                        \
        if (!(expr)) {                                                          \
            ESTD_ASSERTION("Aborting due to (" #expr "): " fmt, ##__VA_ARGS__); \
            abort();                                                            \
        }                                                                       \
    } while (0)

#if (!defined(ESTD_RESULT_IMPLEMENTATION) || defined(ESTD_ALL_IMPLEMENTATION)) && !defined(__ESTD_RESULT_C__)
#define __ESTD_RESULT_C__

ESTD_RESULT_DATA(___ESTD_RESULTS);

#endif

#endif
