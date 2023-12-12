#!/bin/bash

set -xe

BIN=payeq
SRC=payeq.c
CFLAGS="-Wall -Wextra -Werror -pedantic"
# DEBUG_FLAGS="-ggdb"

clang -o $BIN $SRC $CFLAGS
