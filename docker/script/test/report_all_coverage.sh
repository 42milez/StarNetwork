#!/usr/bin/env bash

DOCKER_SCRIPT_DIR=$(dirname $(dirname "${BASH_SOURCE}"))

. "${DOCKER_SCRIPT_DIR}/config.bash"
. "${DOCKER_SCRIPT_DIR}/handler.bash"
. "${DOCKER_SCRIPT_DIR}/var.bash"

llvm-profdata-9 merge -sparse "${WORK_DIR}/default.profraw" -o "${WORK_DIR}/default.profdata"

EXIT_ON_FAIL $? 'llvm-profdata failed'

llvm-cov-9 export "${BUILD_DIR}/bin/all_tests" -instr-profile="${WORK_DIR}/default.profdata" > "${COVERAGE_ALL_PATH}"

EXIT_ON_FAIL $? 'llvm-cov failed'

rm "${WORK_DIR}/default.profraw"
rm "${WORK_DIR}/default.profdata"
