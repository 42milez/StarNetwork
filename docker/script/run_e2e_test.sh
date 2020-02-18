#!/bin/bash

WORK_DIR=/var/app
CMAKE_BUILD_DIR="${WORK_DIR}/cmake-build-test"
TEST_RESULT_DIR="${WORK_DIR}/test-results"

REPORTER=console

if [[ -n ${CATCH_REPORTER} ]]; then
  REPORTER=${CATCH_REPORTER}
fi

if [[ ${REPORTER} = "junit" ]]; then
  "${CMAKE_BUILD_DIR}"/bin/basic_connection_test      -r junit -d yes --order lex -o "${TEST_RESULT_DIR}/e2e_basic_connection_test.xml"
  "${CMAKE_BUILD_DIR}"/bin/broadcast_test             -r junit -d yes --order lex -o "${TEST_RESULT_DIR}/e2e_broadcast_test.xml"
  "${CMAKE_BUILD_DIR}"/bin/send_reliable_command_test -r junit -d yes --order lex -o "${TEST_RESULT_DIR}/e2e_send_reliable_command_test.xml"
else
  "${CMAKE_BUILD_DIR}"/bin/basic_connection_test      -r console -d yes --order lex
  "${CMAKE_BUILD_DIR}"/bin/broadcast_test             -r console -d yes --order lex
  "${CMAKE_BUILD_DIR}"/bin/send_reliable_command_test -r console -d yes --order lex
fi
