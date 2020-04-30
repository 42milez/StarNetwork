#!/bin/bash

SRC_DIR=$(dirname "${BASH_SOURCE}")

. "${SRC_DIR}/build_base.bash"

configure ''
build 'p2p_techdemo'
