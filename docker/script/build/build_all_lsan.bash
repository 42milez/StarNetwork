#!/bin/bash

SRC_DIR=$(dirname "${BASH_SOURCE[0]}")

# shellcheck source=./build_base.bash
. "${SRC_DIR}/build_base.bash"

configure 'leak'
build 'all'
