#!/bin/bash

WORK_DIR=/tmp
CMAKE_BUILD_DIR="${WORK_DIR}/build/cmake-build-test"
TEST_RESULT_DIR="${WORK_DIR}/test-results"

REPORTER=console

if [[ -z ${CATCH_REPORTER} ]]; then
  REPORTER=${CATCH_REPORTER}
fi

"${CMAKE_BUILD_DIR}"/test/unit/lib/rudp/command/rudp_command_test -r "${REPORTER}" -d yes --order lex -o "${TEST_RESULT_DIR}/unit_rudp_command_test.xml"
