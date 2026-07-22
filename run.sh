#!/usr/bin/env sh

cc test.c -lm -std=c11 -o test -g3 -Wall -Wextra -Wpedantic -pedantic

if [ $? -eq 0 ]; then
    ./test "$@"
fi
