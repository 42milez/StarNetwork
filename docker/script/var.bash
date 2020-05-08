#!/bin/bash

export WORK_DIR=/var/app

export CMAKE_C_COMPILER=/usr/bin/clang-9
export CMAKE_CXX_COMPILER=/usr/bin/clang++-9
export CMAKE_CXX_FLAGS="${CXX_FLAGS:-}"
export CMAKE_CXX_FLAGS_DEBUG="${CXX_FLAGS_DEBUG:--DDEBUG -g -O0}"
export CMAKE_CXX_FLAGS_RELEASE="${CXX_FLAGS_RELEASE:--g -O1}"
export CMAKE_GENERATOR='CodeBlocks - Unix Makefiles'

export GRCOV_LOCATION='https://github.com/mozilla/grcov/releases/download'
export GRCOV_VERSION='v0.5.9'
export GRCOV_ASSET='grcov-linux-x86_64.tar.bz2'
