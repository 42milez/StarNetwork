#!/bin/bash

DOCKER_SCRIPT_DIR=$(dirname "$(dirname "${BASH_SOURCE}")")
PERF='/usr/lib/linux-tools/4.15.0-99-generic/perf'
TMP_DIR='/var/app/tmp'

mkdir -p "${TMP_DIR}"

# shellcheck source=../config.bash
. "${DOCKER_SCRIPT_DIR}/config.bash"

# shellcheck source=../var.bash
. "${DOCKER_SCRIPT_DIR}/var.bash"

git clone https://github.com/brendangregg/FlameGraph /root/FlameGraph

cd /root/FlameGraph || exit

cp "${BUILD_DIR}/bin/all_tests" .

"${PERF}" record -F 99 -a -g ./all_tests
"${PERF}" script | ./stackcollapse-perf.pl > "${TMP_DIR}/out.perf-folded"

./flamegraph.pl "${TMP_DIR}/out.perf-folded" > "${TMP_DIR}/perf-kernel.svg"
