#!/usr/bin/env bash

export WORK_DIR=/var/app

export CMAKE_BUILD_TYPE=Debug
export CMAKE_C_COMPILER=/usr/bin/clang-9
export CMAKE_CXX_COMPILER=/usr/bin/clang++-9
export CMAKE_CXX_FLAGS=''
export CMAKE_CXX_FLAGS_DEBUG='-DDEBUG -g -O0'
export CMAKE_GENERATOR='CodeBlocks - Unix Makefiles'

export GRCOV_LOCATION='https://github.com/mozilla/grcov/releases/download'
export GRCOV_VERSION='v0.5.9'
export GRCOV_ASSET='grcov-linux-x86_64.tar.bz2'
