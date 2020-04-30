#!/bin/bash

DOCKER_SCRIPT_DIR=$(dirname $(dirname "${BASH_SOURCE}"))

. "${DOCKER_SCRIPT_DIR}/config.bash"
. "${DOCKER_SCRIPT_DIR}/var.bash"

REPORTER=console

if [[ -n ${CATCH_REPORTER} ]]; then
  REPORTER=${CATCH_REPORTER}
fi

if [[ ${REPORTER} == "junit" ]]; then
  "${BUILD_DIR}"/bin/all_tests -r junit -d yes --order lex -o "${TEST_RESULT_DIR}/all_tests.xml"
else
  "${BUILD_DIR}"/bin/all_tests -r console -d yes --order lex
fi
