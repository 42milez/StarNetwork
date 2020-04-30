#!/bin/bash

SRC_DIR=$(dirname "${BASH_SOURCE}")

. "${SRC_DIR}/build_base.bash"

configure 'address'
build 'all'
