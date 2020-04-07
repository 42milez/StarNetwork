#!/usr/bin/env bash

COLOR_ERROR="\033[0;31m[ERROR]\033[0;39m"
COLOR_INFO="\033[0;32m[INFO]\033[0;39m"
COLOR_WARN="\033[0;33m[WARN]\033[0;39m"

CONSOLE() {
  CALLER=${FUNCNAME[1]}

  if [[ ${#} -ne 2 ]]; then
    echo -e "${COLOR_ERROR} ${CALLER} requires 1 argument."
    exit 1
  fi

  MESSAGE=${1}
  WITHOUT_LINE_BREAK=${2}

  if [[ -z "${WITHOUT_LINE_BREAK}" ]]; then
    WITHOUT_LINE_BREAK=false
  fi

  if "${WITHOUT_LINE_BREAK}"; then
    echo -e -n "${MESSAGE}"
  else
    echo -e "${MESSAGE}"
  fi
}

CONSOLE_ERROR() {
  MESSAGE=${1}
  CONSOLE "${COLOR_ERROR} ${MESSAGE}" false
}

CONSOLE_ERROR_NOBR() {
  MESSAGE=${1}
  CONSOLE "${COLOR_ERROR} ${MESSAGE}" true
}

CONSOLE_INFO() {
  MESSAGE=${1}
  CONSOLE "${COLOR_INFO} ${MESSAGE}" false
}

CONSOLE_INFO_NOBR() {
  MESSAGE=${1}
  CONSOLE "${COLOR_INFO} ${MESSAGE}" true
}

CONSOLE_PLAIN() {
  MESSAGE=${1}
  CONSOLE "${MESSAGE}" false
}

CONSOLE_PLAIN_NOBR() {
  MESSAGE=${1}
  CONSOLE "${MESSAGE}" true
}

CONSOLE_WARN() {
  MESSAGE=${1}
  CONSOLE "${COLOR_WARN} ${MESSAGE}" false
}

CONSOLE_WARN_NOBR() {
  MESSAGE=${1}
  CONSOLE "${COLOR_WARN} ${MESSAGE}" true
}

# https://stackoverflow.com/questions/2575037/how-to-get-the-cursor-position-in-bash
CURSOR_HORIZONTAL_POSITION() {
  exec </dev/tty
  oldstty=$(stty -g)
  stty raw -echo min 0
  echo -en "\033[6n" >/dev/tty
  IFS=';' read -r -d R -a pos
  stty $oldstty
  row=$((${pos[0]:2} - 1)) # strip off the esc-[
  col=$((${pos[1]} - 1))

  echo "${row}"
}

LINE_BREAK_IF_CURSOR_MOVED() {
  if [[ $(CURSOR_HORIZONTAL_POSITION) -ne 0 ]]; then
    {
      echo ''
    }
  fi
}

LOG() {
  MESSAGE=${1}
  LOG_FILE_PATH=${2}

  echo "[$(date)] ${MESSAGE}" >>"${LOG_FILE_PATH}"
}
