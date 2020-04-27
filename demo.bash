#!/usr/bin/env bash

TMUX_SESSION_NAME='demo'

LOCAL_WORK_DIR="${HOME}/Workspace/p2p-techdemo"
REMOTE_WORK_DIR="/root"

LONG_SLEEP_SEC=2
SHORT_SLEEP_SEC=0.5

CLIENT1_PAIN=0.0
CLIENT2_PAIN=0.1
SERVER_PAIN=0.2

long_sleep()
{
  sleep ${LONG_SLEEP_SEC}
}

short_sleep()
{
  sleep ${SHORT_SLEEP_SEC}
}

tmux_move_pain()
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
  tmux_move_pain "${CLIENT1_PAIN}"               ; short_sleep
  tmux split-window -h                           ; short_sleep
}

DOCKER_COMPOSE_FILE='docker-compose.release.yml'
DOCKER_SERVICE_NAME='demo'

: 'RUN CONTAINERS' &&
{
  docker-compose -f docker-compose.release.yml up -d demo

  tmux_move_pain "${CLIENT1_PAIN}"                                                                  ; short_sleep
  tmux_run_command "cd ${LOCAL_WORK_DIR}"                                                           ; short_sleep
  tmux_run_command "docker-compose -f ${DOCKER_COMPOSE_FILE} exec ${DOCKER_SERVICE_NAME} /bin/bash" ; long_sleep
  tmux_run_command "cd ${REMOTE_WORK_DIR}"                                                          ; short_sleep
  tmux_run_command 'clear'                                                                          ; short_sleep

  tmux_move_pain "${CLIENT2_PAIN}"                                                                  ; short_sleep
  tmux_run_command "cd ${LOCAL_WORK_DIR}"                                                           ; short_sleep
  tmux_run_command "docker-compose -f ${DOCKER_COMPOSE_FILE} exec ${DOCKER_SERVICE_NAME} /bin/bash" ; long_sleep
  tmux_run_command "cd ${REMOTE_WORK_DIR}"                                                          ; short_sleep
  tmux_run_command 'clear'                                                                          ; short_sleep

  tmux_move_pain "${SERVER_PAIN}"                                                                   ; short_sleep
  tmux_run_command "cd ${LOCAL_WORK_DIR}"                                                           ; short_sleep
  tmux_run_command "docker-compose -f ${DOCKER_COMPOSE_FILE} exec ${DOCKER_SERVICE_NAME} /bin/bash" ; long_sleep
  tmux_run_command "cd ${REMOTE_WORK_DIR}"                                                          ; short_sleep
  tmux_run_command 'clear'                                                                          ; short_sleep
}

BINARY_NAME='p2p_techdemo'

: 'RUN SERVER' &&
{
  tmux_move_pain "${SERVER_PAIN}"                   ; short_sleep
  tmux_run_command "./${BINARY_NAME} --mode server" ; long_sleep
}

CLIENT1_PORT=49153
CLIENT2_PORT=49154

: 'RUN CLIENTS' &&
{
  tmux_move_pain "${CLIENT1_PAIN}"                                         ; short_sleep
  tmux_run_command "./${BINARY_NAME} --mode client --port ${CLIENT1_PORT}" ; long_sleep

  tmux_move_pain "${CLIENT2_PAIN}"                                         ; short_sleep
  tmux_run_command "./${BINARY_NAME} --mode client --port ${CLIENT2_PORT}" ; long_sleep
}

: 'SEND A MESSAGE FROM CLIENT1 (1/2)' &&
{
  tmux_move_pain "${CLIENT1_PAIN}" ; short_sleep
  tmux_run_command "hello1"        ; short_sleep
}

: 'SEND A MESSAGE FROM CLIENT2 (1/2)' &&
{
  tmux_move_pain "${CLIENT2_PAIN}" ; short_sleep
  tmux_run_command "hello2"        ; short_sleep
}

: 'SEND A MESSAGE FROM CLIENT1 (2/2)' &&
{
  tmux_move_pain "${CLIENT1_PAIN}" ; short_sleep
  tmux_run_command "hello3"        ; short_sleep
}

: 'SEND A MESSAGE FROM CLIENT2 (2/2)' &&
{
  tmux_move_pain "${CLIENT2_PAIN}" ; short_sleep
  tmux_run_command "hello4"        ; short_sleep
}
