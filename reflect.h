#ifndef __ESTD_REFLECT_H__
#define __ESTD_REFLECT_H__

#include "meta.h"
#define ___ESTD_UNION_ENUM_CASE(ctx, T, name) ___##ctx##_##name
#define ___ESTD_UNION_ENUM_SEP ,

#define ___ESTD_UNION_DATA_CASE(ctx, T, name) T ___##name;
#define ___ESTD_UNION_DATA_SEP

#define ESTD_DECLARE_UNION(T)                                                                             \
    typedef enum ___##T##Type{T##Cases(T, ___ESTD_UNION_ENUM_CASE, ___ESTD_UNION_ENUM_SEP)} ___##T##Type; \
    typedef union ___##T##Data {                                                                          \
        T##Cases(T, ___ESTD_UNION_DATA_CASE, ___ESTD_UNION_DATA_SEP)                                      \
    } ___##T##Data;                                                                                       \
    typedef struct T {                                                                                    \
        ___##T##Type ___type;                                                                             \
        ___##T##Data ___data;                                                                             \
    } T;

#endif
