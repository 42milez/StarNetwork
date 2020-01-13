#!/bin/bash

WORK_DIR=/tmp
BUILD_DIR="${WORK_DIR}/build"
TEST_RESULT_DIR=/tmp/test-results

REPORTER=console

if [[ -z ${CATCH_REPORTER} ]]; then
  REPORTER=${CATCH_REPORTER}
fi

"${BUILD_DIR}"/test/e2e/rudp/basic_connection/basic_connection_test           -r "${REPORTER}" -d yes --order lex -o "${TEST_RESULT_DIR}/e2e/rudp/basic_connection/basic_connection_test.xml"
"${BUILD_DIR}"/test/e2e/rudp/broadcast/babroadcast_test                       -r "${REPORTER}" -d yes --order lex -o "${TEST_RESULT_DIR}/e2e/rudp/broadcast/babroadcast_test.xml"
"${BUILD_DIR}"/test/e2e/rudp/send_reliable_command/send_reliable_command_test -r "${REPORTER}" -d yes --order lex -o "${TEST_RESULT_DIR}/e2e/rudp/send_reliable_command/send_reliable_command_test.xml"
