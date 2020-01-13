#!/bin/bash

WORK_DIR=/tmp
CMAKE_BUILD_DIR="${WORK_DIR}/build/cmake-build-test"
TEST_RESULT_DIR="${WORK_DIR}/test-results"

REPORTER=console

if [[ -n ${CATCH_REPORTER} ]]; then
  REPORTER=${CATCH_REPORTER}
fi

"${CMAKE_BUILD_DIR}"/test/e2e/rudp/basic_connection/basic_connection_test           -r "${REPORTER}" -d yes --order lex -o "${TEST_RESULT_DIR}/e2e_basic_connection_test.xml"
"${CMAKE_BUILD_DIR}"/test/e2e/rudp/broadcast/babroadcast_test                       -r "${REPORTER}" -d yes --order lex -o "${TEST_RESULT_DIR}/e2e_babroadcast_test.xml"
"${CMAKE_BUILD_DIR}"/test/e2e/rudp/send_reliable_command/send_reliable_command_test -r "${REPORTER}" -d yes --order lex -o "${TEST_RESULT_DIR}/e2e_send_reliable_command_test.xml"
