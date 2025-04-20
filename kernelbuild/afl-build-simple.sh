#!/bin/bash
bazel clean
CC=afl-clang-lto CXX=afl-clang-lto++ RANLIB=llvm-ranlib AR=llvm-ar AS=llvm-as bazel build //main:parser
cp bazel-bin/main/parser parser.afl
