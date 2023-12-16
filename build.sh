#!/bin/bash

set -xe

BIN=payeq
SRC=payeq.c
CFLAGS="-Wall -Wextra -pedantic"
DEBUG_FLAGS="-ggdb -fsanitize=address,undefined -fanalyzer"

if [ "$1" == "--debug" ]; then
    gcc -o $BIN $SRC $CFLAGS $DEBUG_FLAGS
else
    gcc -o $BIN $SRC $CFLAGS
fi
