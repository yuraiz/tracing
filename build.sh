#!/usr/bin/env bash

mkdir -p target

link_flags="-F /System/Library/PrivateFrameworks -lreadline -framework CoreSymbolication"

warning_flags="-Wall -Wextra -Wpedantic -Wconversion -Wdouble-promotion -Wno-unused-parameter\
    -Wno-unused-function -Wno-sign-conversion -Wno-flexible-array-extensions -Wno-macro-redefined"

debug_flags="-g -fsanitize=undefined -fsanitize-trap -fsanitize=address"

clang src/tracer_main.c -std=c11 -g $link_flags $warning_flags $debug_flags -o target/tracer

# Build sample programs
as code.s -o target/test.o && ld -o target/test target/test.o -l System -syslibroot `xcrun -sdk macosx --show-sdk-path`
rm target/test.o

clang simple_app.c -o target/simple_app