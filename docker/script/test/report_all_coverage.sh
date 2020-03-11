#!/usr/bin/env bash

"${BUILD_DIR}"/bin/all_tests

llvm-profdata-9 merge -sparse "${BUILD_DIR}/bin/default.profraw" -o "${BUILD_DIR}/default.profdata"

llvm-cov-9 export "${BUILD_DIR}/bin/all_tests" -instr-profile="${BUILD_DIR}/default.profdata" > "${COVERAGE_ALL_PATH}"
