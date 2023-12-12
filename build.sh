#!/bin/bash

set -xe

BIN=payeq
SRC=main.c
CFLAGS="-Wall -Wextra -Werror -pedantic"
# DEBUG_FLAGS="-ggdb"

clang -o $BIN $SRC $CFLAGS
