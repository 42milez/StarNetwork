#!/usr/bin/env bash

DOCKER_SCRIPT_DIR=$(dirname $(dirname "${BASH_SOURCE}"))

. "${DOCKER_SCRIPT_DIR}/config.bash"
. "${DOCKER_SCRIPT_DIR}/var.bash"

"${BUILD_DIR}"/bin/all_tests >/dev/null 2>&1

llvm-profdata-9 merge -sparse "${WORK_DIR}/default.profraw" -o "${WORK_DIR}/default.profdata"
llvm-cov-9 export "${BUILD_DIR}/bin/all_tests" -instr-profile="${WORK_DIR}/default.profdata" > "${COVERAGE_ALL_PATH}"

rm "${WORK_DIR}/default.profraw"
rm "${WORK_DIR}/default.profdata"
