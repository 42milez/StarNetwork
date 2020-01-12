#!/bin/bash

WORKING_DIRECTORY=${HOME}
BUILD_DIRECTORY="${WORKING_DIRECTORY}/build"

"${BUILD_DIRECTORY}"/test/unit/lib/rudp/command/rudp_command_test -r console -d yes --order lex
