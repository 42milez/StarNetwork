#!/bin/bash

WORK_DIR=/var/app
CMAKE_BUILD_DIR="${WORK_DIR}/cmake-build-test"
TEST_RESULT_DIR="${WORK_DIR}/test-results"

REPORTER=console

if [[ -n ${CATCH_REPORTER} ]]; then
  REPORTER=${CATCH_REPORTER}
fi

if [[ ${REPORTER} = "junit" ]]; then
  "${CMAKE_BUILD_DIR}"/bin/command_pod_test -r junit -d yes --order lex -o "${TEST_RESULT_DIR}/unit_command_pod_test.xml"
  "${CMAKE_BUILD_DIR}"/bin/network_test     -r junit -d yes --order lex -o "${TEST_RESULT_DIR}/unit_network_test.xml"
  "${CMAKE_BUILD_DIR}"/bin/ip_address_test  -r junit -d yes --order lex -o "${TEST_RESULT_DIR}/unit_ip_address_test.xml"
else
  "${CMAKE_BUILD_DIR}"/bin/command_pod_test -r console -d yes --order lex
  "${CMAKE_BUILD_DIR}"/bin/network_test     -r console -d yes --order lex
  "${CMAKE_BUILD_DIR}"/bin/ip_address_test  -r console -d yes --order lex
fi
