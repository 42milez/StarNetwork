#!/usr/bin/env bash

TMUX_SESSION_NAME='demo'

LOCAL_WORK_DIR="${HOME}/Workspace/p2p-techdemo"
REMOTE_WORK_DIR="/root"

LONG_SLEEP_SEC=2
SHORT_SLEEP_SEC=0.5

CLIENT1_PAIN=0.0
CLIENT2_PAIN=0.1
SERVER_PAIN=0.2

: 'SPLIT WINDOW INTO 3 PAINS' &&
{
    tmux split-window -v -t "${TMUX_SESSION_NAME}"             ; sleep ${SHORT_SLEEP_SEC}
    tmux select-pane -t "${TMUX_SESSION_NAME}:${CLIENT1_PAIN}" ; sleep ${SHORT_SLEEP_SEC}
    tmux split-window -h                                       ; sleep ${SHORT_SLEEP_SEC}
}

: 'RUN DOCKER CONTAINERS' &&
{
    tmux select-pane -t "${TMUX_SESSION_NAME}:${CLIENT1_PAIN}"                                              ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "cd ${LOCAL_WORK_DIR}" Enter                                   ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" 'dc -f docker-compose.dev.yml exec dev_server /bin/bash' Enter ; sleep ${LONG_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "cd ${REMOTE_WORK_DIR}" Enter                                  ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" 'clear' Enter                                                  ; sleep ${SHORT_SLEEP_SEC}

    tmux select-pane -t "${TMUX_SESSION_NAME}:${CLIENT2_PAIN}"                                              ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "cd ${LOCAL_WORK_DIR}" Enter                                   ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" 'dc -f docker-compose.dev.yml exec dev_server /bin/bash' Enter ; sleep ${LONG_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "cd ${REMOTE_WORK_DIR}" Enter                                  ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" 'clear' Enter                                                  ; sleep ${SHORT_SLEEP_SEC}

    tmux select-pane -t "${TMUX_SESSION_NAME}:${SERVER_PAIN}"                                               ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "cd ${LOCAL_WORK_DIR}" Enter                                   ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" 'dc -f docker-compose.dev.yml exec dev_server /bin/bash' Enter ; sleep ${LONG_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "cd ${REMOTE_WORK_DIR}" Enter                                  ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" 'clear' Enter                                                  ; sleep ${SHORT_SLEEP_SEC}
}

BINARY_NAME='p2p_techdemo'

: 'RUN SERVER' &&
{
    tmux select-pane -t "${TMUX_SESSION_NAME}:${SERVER_PAIN}"                       ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "./${BINARY_NAME} --mode server" Enter ; sleep 2
}

CLIENT1_PORT=49153
CLIENT2_PORT=49154

: 'RUN CLIENTS' &&
{
    tmux select-pane -t "${TMUX_SESSION_NAME}:${CLIENT1_PAIN}"                                             ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "./${BINARY_NAME} --mode client --port ${CLIENT1_PORT}" Enter ; sleep ${LONG_SLEEP_SEC}

    tmux select-pane -t "${TMUX_SESSION_NAME}:${CLIENT2_PAIN}"                                             ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "./${BINARY_NAME} --mode client --port ${CLIENT2_PORT}" Enter ; sleep ${LONG_SLEEP_SEC}
}

: 'SEND A MESSAGE FROM CLIENT1 (1/2)' &&
{
    tmux select-pane -t "${TMUX_SESSION_NAME}:${CLIENT1_PAIN}" ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "hello1" Enter    ; sleep ${LONG_SLEEP_SEC}
}

: 'SEND A MESSAGE FROM CLIENT2 (1/2)' &&
{
    tmux select-pane -t "${TMUX_SESSION_NAME}:${CLIENT2_PAIN}" ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "hello2" Enter    ; sleep ${LONG_SLEEP_SEC}
}

: 'SEND A MESSAGE FROM CLIENT1 (2/2)' &&
{
    tmux select-pane -t "${TMUX_SESSION_NAME}:${CLIENT1_PAIN}" ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "hello3" Enter    ; sleep ${LONG_SLEEP_SEC}
}

: 'SEND A MESSAGE FROM CLIENT2 (2/2)' &&
{
    tmux select-pane -t "${TMUX_SESSION_NAME}:${CLIENT2_PAIN}" ; sleep ${SHORT_SLEEP_SEC}
    tmux send-keys -t "${TMUX_SESSION_NAME}" "hello4" Enter    ; sleep ${LONG_SLEEP_SEC}
}
