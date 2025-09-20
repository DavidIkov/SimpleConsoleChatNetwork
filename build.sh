#! /bin/bash

cmake -S. -Bbuild \
    -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DANTLR4_INCLUDE_DIR="/usr/include/antlr4-runtime" -DANTLR4_SHARED_LIB="/lib/x86_64-linux-gnu/libantlr4-runtime.so.4.9" \
    -DANTLR4_BINARY="antlr4" \
    -DCMAKE_GENERATOR="Ninja"

