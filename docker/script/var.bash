#!/usr/bin/env bash

export WORK_DIR=/var/app
export COVERAGE_DIR="${WORK_DIR}/coverage"

export CMAKE_BUILD_TYPE=Debug
export CMAKE_C_COMPILER=/usr/bin/clang-9
export CMAKE_CXX_COMPILER=/usr/bin/clang++-9
export CMAKE_CXX_FLAGS=''
export CMAKE_CXX_FLAGS_DEBUG='-DDEBUG -g -O0'
export CMAKE_GENERATOR='CodeBlocks - Unix Makefiles'
