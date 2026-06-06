#ifndef __ESTD_ARGS_H__
#define __ESTD_ARGS_H__

#include <iso646.h>
#include <stdbool.h>

#include "str.h"

#define ESTD_EVAL(...) __VA_ARGS__

#define ESTD_ARGS_NAMED_HELP_GENERATOR(name, description, action) "\t--" #name "=\t" description "\n"
#define ESTD_ARGS_NAMED_HELP_SEPARATOR

#define ESTD_ARGS_POSITIONAL_HELP_GENERATOR(name, description, action) "\t" #name "\t" description "\n"
#define ESTD_ARGS_POSITIONAL_HELP_SEPARATOR

#define ESTD_ARGS_HELP_MESSAGE(    \
    required_named_arguments,      \
    optional_named_arguments,      \
    required_positional_arguments, \
    optional_positional_arguments  \
)                                  \
    "Usage: %s [named arguments] (required positional arguments) [optional positional arguments]\n" \
    "Required Named Arguments:\n" \
    required_named_arguments(ESTD_ARGS_NAMED_HELP_GENERATOR, ESTD_ARGS_NAMED_HELP_SEPARATOR) \
    "Required Positional Arguments:\n" \
    required_positional_arguments(ESTD_ARGS_POSITIONAL_HELP_GENERATOR, ESTD_ARGS_POSITIONAL_HELP_SEPARATOR) \
    "Optional Named Arguments:\n" \
    optional_named_arguments(ESTD_ARGS_NAMED_HELP_GENERATOR, ESTD_ARGS_NAMED_HELP_SEPARATOR) \
    "Optional Positional Arguments:\n" \
    optional_positional_arguments(ESTD_ARGS_POSITIONAL_HELP_GENERATOR, ESTD_ARGS_POSITIONAL_HELP_SEPARATOR)

#define ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_GENERATOR(name, description, action) bool __estd_seen_##name = false;
#define ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_SEPARATOR

#define ESTD_ARGS_REQUIRED_VARIABLE_CHECK_GENERATOR(name, description, action) \
    if (not __estd_seen_##name) {                                              \
        ESTD_THROW(ESTD_MISSING_ARG, "Missing argument %s", #name);            \
    }
#define ESTD_ARGS_REQUIRED_VARIABLE_CHECK_SEPARATOR

#define ESTD_ARGS_REQUIRED_NAMED_CHECKER_GENERATOR(name, description, action) \
    if (estd_string_compare(key, ESTD_LITERAL(#name)) == 0) {                 \
        __estd_seen_##name = true;                                            \
        ESTD_EVAL action;                                                     \
    }
#define ESTD_ARGS_REQUIRED_NAMED_CHECKER_SEPARATOR else

#define ESTD_ARGS_OPTIONAL_NAMED_CHECKER_GENERATOR(name, description, action) \
    if (estd_string_compare(key, ESTD_LITERAL(#name)) == 0) {                 \
        ESTD_EVAL action;                                                     \
    }
#define ESTD_ARGS_OPTIONAL_NAMED_CHECKER_SEPARATOR else

#define ESTD_ARGS_REQUIRED_POSITIONAL_CHECKER_GENERATOR(name, description, action)                               \
    if (position - __estd_positional_start + 1 ==                                                                \
        __COUNTER__ - __estd_positional_counter_start) { /* +1 is needed because __COUNTER__ hack starts at 0 */ \
        __estd_seen_##name = true;                                                                               \
        ESTD_EVAL action;                                                                                        \
    }
#define ESTD_ARGS_REQUIRED_POSITIONAL_CHECKER_SEPARATOR else

#define ESTD_ARGS_OPTIONAL_POSITIONAL_CHECKER_GENERATOR(name, description, action)                 \
    if (position - __estd_positional_start + 1 == __COUNTER__ - __estd_positional_counter_start) { \
        ESTD_EVAL action;                                                                          \
    }
#define ESTD_ARGS_OPTIONAL_POSITIONAL_CHECKER_SEPARATOR else

#define ESTD_GENERATE_ARGUMENT_PARSER(                                     \
    name,                                                                  \
    params,                                                                \
    required_named_arguments,                                              \
    optional_named_arguments,                                              \
    required_positional_arguments,                                         \
    optional_positional_arguments                                          \
)                                                                          \
    EstdResult name(int argc, char* argv[argc], ESTD_EVAL params) {        \
        if (argc == 1) {                                                   \
            printf(                                                        \
                ESTD_ARGS_HELP_MESSAGE(                                    \
                    required_named_arguments,                              \
                    optional_named_arguments,                              \
                    required_positional_arguments,                         \
                    optional_positional_arguments                          \
                ),                                                         \
                argv[0]                                                    \
            );                                                             \
            return ESTD_SUCCESS;                                           \
        }                                                                  \
        required_named_arguments(                                          \
            ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_GENERATOR,             \
            ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_SEPARATOR              \
        );                                                                 \
        required_positional_arguments(                                     \
            ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_GENERATOR,             \
            ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_SEPARATOR              \
        );                                                                 \
        int position;                                                      \
        int __estd_positional_start = 0;                                   \
        for (position = 1; position < argc; position++) {                  \
            EstdString arg = ESTD_CTRING(argv[position]);                  \
            if (not estd_string_has_prefix(arg, ESTD_LITERAL("--"))) {     \
                __estd_positional_start = position;                        \
                break;                                                     \
            }                                                              \
            EstdString __estd_shifted = ESTD_SLICE(arg, 2, arg.length);    \
            EstdString value = __estd_shifted;                             \
            EstdString key = estd_string_split(&value, ESTD_LITERAL("=")); \
            required_named_arguments(                                      \
                ESTD_ARGS_REQUIRED_NAMED_CHECKER_GENERATOR,                \
                ESTD_ARGS_REQUIRED_NAMED_CHECKER_SEPARATOR                 \
            );                                                             \
            optional_named_arguments(                                      \
                ESTD_ARGS_OPTIONAL_NAMED_CHECKER_GENERATOR,                \
                ESTD_ARGS_OPTIONAL_NAMED_CHECKER_SEPARATOR                 \
            );                                                             \
        }                                                                  \
        int __estd_positional_counter_start = __COUNTER__;                 \
        for (; position < argc; position++) {                              \
            EstdString arg = ESTD_CTRING(argv[position]);                  \
            required_positional_arguments(                                 \
                ESTD_ARGS_REQUIRED_POSITIONAL_CHECKER_GENERATOR,           \
                ESTD_ARGS_REQUIRED_POSITIONAL_CHECKER_SEPARATOR            \
            );                                                             \
            optional_positional_arguments(                                 \
                ESTD_ARGS_OPTIONAL_POSITIONAL_CHECKER_GENERATOR,           \
                ESTD_ARGS_OPTIONAL_POSITIONAL_CHECKER_SEPARATOR            \
            );                                                             \
        }                                                                  \
        required_named_arguments(                                          \
            ESTD_ARGS_REQUIRED_VARIABLE_CHECK_GENERATOR,                   \
            ESTD_ARGS_REQUIRED_VARIABLE_CHECK_SEPARATOR                    \
        );                                                                 \
        required_positional_arguments(                                     \
            ESTD_ARGS_REQUIRED_VARIABLE_CHECK_GENERATOR,                   \
            ESTD_ARGS_REQUIRED_VARIABLE_CHECK_SEPARATOR                    \
        );                                                                 \
        return ESTD_SUCCESS;                                               \
    }

#define ESTD_NO_ARGUMENT(ARG, SEP)

#endif