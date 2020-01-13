#!/bin/bash

WORK_DIR=/tmp
TEST_DIR="${WORK_DIR}/test"
BUILD_DIR="${WORK_DIR}/cmake-build"
TEST_RESULT_DIR="${WORK_DIR}/test-results"

REPORTER=console

if [[ -z ${CATCH_REPORTER} ]]; then
  REPORTER=${CATCH_REPORTER}
fi

"${BUILD_DIR}"/test/unit/lib/rudp/command/rudp_command_test -r "${REPORTER}" -d yes --order lex -o "${TEST_RESULT_DIR}/unit/lib/rudp/command/rudp_command_test.xml"
