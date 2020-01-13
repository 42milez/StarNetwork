#!/bin/bash

WORK_DIR=/tmp/build
BUILD_DIR="${WORK_DIR}/cmake-build"

cd "${BUILD_DIR}" || exit

/usr/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -G "CodeBlocks - Unix Makefiles" "${WORK_DIR}"
/usr/bin/cmake --build "${BUILD_DIR}" --target all -- -j 4
