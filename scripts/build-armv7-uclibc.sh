#!/bin/sh

set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build/armv7-uclibc}"
BUILD_JOBS="${BUILD_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)}"

export CROSS_COMPILE="${CROSS_COMPILE:-arm-rockchip830-linux-uclibcgnueabihf-}"

cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${PROJECT_ROOT}/cmake/toolchains/armv7-uclibc.cmake" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" -j"${BUILD_JOBS}"
