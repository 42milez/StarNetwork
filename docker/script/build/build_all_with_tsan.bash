#!/usr/bin/env bash

SRC_DIR=$(dirname "${BASH_SOURCE}")

. "${SRC_DIR}/_build_base.bash"

configure 'thread'
build 'all'
