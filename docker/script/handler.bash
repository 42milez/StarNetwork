#!/bin/bash

DOCKER_SCRIPT_DIR=$(dirname "${BASH_SOURCE}")

. "${DOCKER_SCRIPT_DIR}/io.bash"

EXIT_ON_FAIL() {
  COMMAND_STATUS=${1}
  MESSAGE=${2}

  if [[ ${COMMAND_STATUS} -ne 0 ]]; then
    {
      if [[ -n "${MESSAGE}" ]]; then
        {
          LINE_BREAK_IF_CURSOR_MOVED
          CONSOLE_ERROR "${MESSAGE}"
        }
      fi

      exit 1
    }
  fi
}

EXIT_IF_ARGC_INVALID() {
  ARGC_REQUIRED=${1}
  ARGC_PASSED=${2}
  CALLER=${FUNCNAME[1]}

  if [[ ${ARGC_REQUIRED} -ne ${ARGC_PASSED} ]]; then
    {
      if [[ ${ARGC_REQUIRED} -eq 1 ]]; then
        {
          CONSOLE_ERROR "${CALLER} requires 1 argument."
        }
      else
        {
          CONSOLE_ERROR "${CALLER} requires ${ARGC_REQUIRED} arguments."
        }
      fi

      exit 1
    }
  fi
}
