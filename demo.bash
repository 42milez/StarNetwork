#!/bin/bash

LOCAL_WORK_DIR="${HOME}/Workspace/p2p-techdemo"
REMOTE_WORK_DIR="/root"

TMUX_SESSION_NAME='demo'
TMUX_GUEST1_PAIN=0.0
TMUX_GUEST2_PAIN=0.1
TMUX_HOST_PAIN=0.2

LONG_SLEEP_SEC=2
SHORT_SLEEP_SEC=0.5

DOCKER_COMPOSE_FILE='docker-compose.release.yml'
DOCKER_SERVICE_NAME='demo'

BINARY='/var/app/cmake-build-release/bin/p2p_techdemo'
GUEST1_PORT=49153
GUEST2_PORT=49154

long_sleep()
{
  sleep ${LONG_SLEEP_SEC}
}

short_sleep()
{
  sleep ${SHORT_SLEEP_SEC}
}

tmux_switch_pain()
{
  PAIN=${1}
  tmux select-pane -t "${TMUX_SESSION_NAME}:${PAIN}"
}

tmux_run_command()
{
  COMMAND="${1}"
  tmux send-keys -t "${TMUX_SESSION_NAME}" "${COMMAND}" Enter
}

: 'SPLIT WINDOW INTO 3 PAINS' &&
{
  tmux split-window -v -t "${TMUX_SESSION_NAME}" ; short_sleep
  tmux_switch_pain "${TMUX_GUEST1_PAIN}"         ; short_sleep
  tmux split-window -h                           ; short_sleep
}

: 'START CONTAINER' &&
{
  docker-compose -f "${DOCKER_COMPOSE_FILE}" \
    up -d "${DOCKER_SERVICE_NAME}"

  docker-compose -f "${DOCKER_COMPOSE_FILE}" \
    exec "${DOCKER_SERVICE_NAME}" './docker/script/build/build_app.bash'
}

: 'SETUP' &&
{
  # HOST
  tmux_switch_pain "${TMUX_HOST_PAIN}"                                                              ; short_sleep
  tmux_run_command "cd ${LOCAL_WORK_DIR}"                                                           ; short_sleep
  tmux_run_command "docker-compose -f ${DOCKER_COMPOSE_FILE} exec ${DOCKER_SERVICE_NAME} /bin/bash" ; long_sleep
  tmux_run_command "cd ${REMOTE_WORK_DIR}"                                                          ; short_sleep
  tmux_run_command 'clear'                                                                          ; short_sleep

  # GUEST 1
  tmux_switch_pain "${TMUX_GUEST1_PAIN}"                                                            ; short_sleep
  tmux_run_command "cd ${LOCAL_WORK_DIR}"                                                           ; short_sleep
  tmux_run_command "docker-compose -f ${DOCKER_COMPOSE_FILE} exec ${DOCKER_SERVICE_NAME} /bin/bash" ; long_sleep
  tmux_run_command "cd ${REMOTE_WORK_DIR}"                                                          ; short_sleep
  tmux_run_command 'clear'                                                                          ; short_sleep

  # GUEST 2
  tmux_switch_pain "${TMUX_GUEST2_PAIN}"                                                            ; short_sleep
  tmux_run_command "cd ${LOCAL_WORK_DIR}"                                                           ; short_sleep
  tmux_run_command "docker-compose -f ${DOCKER_COMPOSE_FILE} exec ${DOCKER_SERVICE_NAME} /bin/bash" ; long_sleep
  tmux_run_command "cd ${REMOTE_WORK_DIR}"                                                          ; short_sleep
  tmux_run_command 'clear'                                                                          ; short_sleep
}

: 'START PROCESSES' &&
{
  # HOST
  tmux_switch_pain "${TMUX_HOST_PAIN}"                             ; short_sleep
  tmux_run_command "${BINARY} --mode server"                       ; long_sleep

  # GUEST 1
  tmux_switch_pain "${TMUX_GUEST1_PAIN}"                           ; short_sleep
  tmux_run_command "${BINARY} --mode client --port ${GUEST1_PORT}" ; long_sleep

  # GUEST 2
  tmux_switch_pain "${TMUX_GUEST2_PAIN}"                           ; short_sleep
  tmux_run_command "${BINARY} --mode client --port ${GUEST2_PORT}" ; long_sleep
}

: 'SEND A MESSAGE FROM GUEST1 (1/2)' &&
{
  tmux_switch_pain "${TMUX_GUEST1_PAIN}" ; short_sleep
  tmux_run_command "hello1"              ; long_sleep
}

: 'SEND A MESSAGE FROM GUEST2 (1/2)' &&
{
  tmux_switch_pain "${TMUX_GUEST2_PAIN}" ; short_sleep
  tmux_run_command "hello2"              ; long_sleep
}

: 'SEND A MESSAGE FROM GUEST1 (2/2)' &&
{
  tmux_switch_pain "${TMUX_GUEST1_PAIN}" ; short_sleep
  tmux_run_command "hello3"              ; long_sleep
}

: 'SEND A MESSAGE FROM GUEST2 (2/2)' &&
{
  tmux_switch_pain "${TMUX_GUEST2_PAIN}" ; short_sleep
  tmux_run_command "hello4"              ; long_sleep
}

: 'SEND A MESSAGE FROM HOST' &&
{
  tmux_switch_pain "${TMUX_HOST_PAIN}" ; short_sleep
  tmux_run_command "hello5"            ; long_sleep
}

: 'STOP PROCESSES' &&
{
  # GUEST 1
  tmux_switch_pain "${TMUX_GUEST1_PAIN}" ; short_sleep
  tmux_run_command "exit"                ; long_sleep

  # GUEST 2
  tmux_switch_pain "${TMUX_GUEST2_PAIN}" ; short_sleep
  tmux_run_command "exit"                ; long_sleep

  # HOST
  tmux_switch_pain "${TMUX_HOST_PAIN}"  ; short_sleep
  tmux_run_command "exit"               ; long_sleep
}

: 'STOP CONTAINER' &&
{
  docker-compose -f "${DOCKER_COMPOSE_FILE}" down
}
