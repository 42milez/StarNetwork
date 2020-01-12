#!/bin/bash

WORKING_DIRECTORY=/tmp
BUILD_DIRECTORY="${WORKING_DIRECTORY}/build"

cd "${BUILD_DIRECTORY}" || exit

/usr/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -G "CodeBlocks - Unix Makefiles" "${WORKING_DIRECTORY}"
/usr/bin/cmake --build "${BUILD_DIRECTORY}" --target all -- -j 4
