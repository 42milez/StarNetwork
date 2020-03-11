#!/usr/bin/env bash

DOCKER_SCRIPT_DIR=$(dirname $(dirname "${BASH_SOURCE}"))

. "${DOCKER_SCRIPT_DIR}/config.bash"
. "${DOCKER_SCRIPT_DIR}/handler.bash"
. "${DOCKER_SCRIPT_DIR}/var.bash"

wget "${GRCOV_LOCATION}/${GRCOV_VERSION}/${GRCOV_ASSET}"

EXIT_ON_FAIL $? 'Download grcov Failed'

tar xf "${GRCOV_ASSET}"

./grcov --llvm . --token unused --commit-sha unused > "${COVERAGE_FILE_PATH}"
