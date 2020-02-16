#!/usr/bin/env bash

source config/bash.sh
source lib/bash/io.sh

if [[ ${#} -ne 1 ]]; then
  CONSOLE_ERROR 'INVALID ARGUMENT: 1 argument required.'
  exit 1
fi

state=${1}

START_DEV_SERVER_COMMAND='docker-compose run --rm --detach dev_server'
STOP_DEV_SERVER_COMMAND='docker-compose down'

if [[ ${state} = 'start' ]]; then
  if ! eval "${START_DEV_SERVER_COMMAND}"; then
    CONSOLE_ERROR "START DEV-SERVER FAILED"
    exit 1
  fi

  CONSOLE_INFO "DEV-SERVER STARTED"
  exit 0
fi

if [[ ${state} = 'stop' ]]; then
  if ! eval "${STOP_DEV_SERVER_COMMAND}"; then
    CONSOLE_ERROR "STOP DEV-SERVER FAILED"
    exit 1
  fi

  CONSOLE_INFO "DEV-SERVER STOPPED"
  exit 0
fi

CONSOLE_ERROR "INVALID ARGUMENT: Only 'start' or 'stop' are available."
exit 1
