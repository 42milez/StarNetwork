#!/bin/bash

WORK_DIR=/home/dev/app
CMAKE_BUILD_DIR="${WORK_DIR}/cmake-build-test"
TEST_RESULT_DIR="${WORK_DIR}/test-results"

REPORTER=console

if [[ -n ${CATCH_REPORTER} ]]; then
  REPORTER=${CATCH_REPORTER}
fi

if [[ ${REPORTER} = "junit" ]]; then
  "${CMAKE_BUILD_DIR}"/bin/feature_test -r junit -d yes --order lex -o "${TEST_RESULT_DIR}/feature_test.xml"
else
  "${CMAKE_BUILD_DIR}"/bin/feature_test -r console -d yes --order lex
fi
