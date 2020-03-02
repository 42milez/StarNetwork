#!/usr/bin/env bash

export WORK_DIR=/var/app
export BUILD_DIR="${WORK_DIR}/cmake-build-test"

export TEST_RESULT_DIR="${WORK_DIR}/test-results"

export CMAKE_BUILD_TYPE=Debug
export CMAKE_C_COMPILER=/usr/bin/clang-9
export CMAKE_CXX_COMPILER=/usr/bin/clang++-9
export CMAKE_CXX_FLAGS=""
export CMAKE_CXX_FLAGS_DEBUG="-DDEBUG -g -O0"
export GENERATOR="CodeBlocks - Unix Makefiles"
