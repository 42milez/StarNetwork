#!/bin/bash

DOCKER_SCRIPT_DIR=$(dirname "$(dirname "${BASH_SOURCE[0]}")")

# shellcheck source=../config.bash
. "${DOCKER_SCRIPT_DIR}/config.bash"

# shellcheck source=../handler.bash
. "${DOCKER_SCRIPT_DIR}/handler.bash"

# shellcheck source=../io.bash
. "${DOCKER_SCRIPT_DIR}/io.bash"

# shellcheck source=../var.bash
. "${DOCKER_SCRIPT_DIR}/var.bash"

configure() {
  EXIT_IF_ARGC_INVALID 1 "${#}"

  git_modifications=$(git status --porcelain)
  git_remotes=$(git remote -v | grep -q cable)

  if [[ -z ${git_modifications} && -z ${git_remotes} ]]; then
    git remote add cable https://github.com/ethereum/cable
  fi

  if [[ -z ${git_modifications} ]]; then
    git subtree pull --prefix cmake/cable cable master --squash
  fi

  SANITIZER="${1}"

  if [[ "${SANITIZER}" != '' ]]; then
    CMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -fsanitize=${SANITIZER}"
  else
    CMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping"
  fi

  cd "${BUILD_DIR}" || exit 1

  /usr/bin/cmake \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
    -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}" \
    -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}" \
    -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}" \
    "${CMAKE_ADDITIONAL_OPTIONS:-}" \
    -G "${CMAKE_GENERATOR}" \
    "${WORK_DIR}"
}

build() {
  EXIT_IF_ARGC_INVALID 1 "${#}"

  TARGET=${1}

  /usr/bin/cmake --build "${BUILD_DIR}" --target "${TARGET}" -- -j 4
}
