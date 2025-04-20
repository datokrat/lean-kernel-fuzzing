#!/bin/bash
bazel clean
CC=afl-clang-lto CXX=afl-clang-lto++ RANLIB=llvm-ranlib AR=llvm-ar AS=llvm-as bazel build //main:parser
cp bazel-bin/main/parser parser.afl
bazel clean
CC=afl-clang-lto CXX=afl-clang-lto++ RANLIB=llvm-ranlib AR=llvm-ar AS=llvm-as bazel build //main:parser --action_env="AFL_USE_ASAN=1 AFL_USE_UBSAN=1 AFL_USE_CFISAN=1"
cp bazel-bin/main/parser parser.afl-asan
bazel clean
CC=afl-clang-lto CXX=afl-clang-lto++ RANLIB=llvm-ranlib AR=llvm-ar AS=llvm-as bazel build //main:parser --action_env="AFL_LLVM_LAF_ALL=1"
cp bazel-bin/main/parser parser.afl-laf-intel
bazel clean
CC=afl-clang-lto CXX=afl-clang-lto++ RANLIB=llvm-ranlib AR=llvm-ar AS=llvm-as bazel build //main:parser --action_env="AFL_LLVM_CMPLOG=1"
cp bazel-bin/main/parser parser.afl-redqueen
