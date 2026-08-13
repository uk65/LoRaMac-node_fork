#!/usr/bin/env bash

set -euo pipefail

readonly ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly TOOLCHAIN_GCC="$(command -v arm-none-eabi-gcc || true)"

if [[ -z "${TOOLCHAIN_GCC}" ]]; then
    echo "arm-none-eabi-gcc was not found in PATH." >&2
    exit 1
fi

readonly TOOLCHAIN_PREFIX="$(cd "$(dirname "${TOOLCHAIN_GCC}")/.." && pwd)"

build_role() {
    local role="$1"
    local build_dir="${ROOT_DIR}/build/power_${role}"

    cmake \
        -S "${ROOT_DIR}" \
        -B "${build_dir}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="${ROOT_DIR}/cmake/toolchain-arm-none-eabi.cmake" \
        -DTOOLCHAIN_PREFIX="${TOOLCHAIN_PREFIX}" \
        -DAPPLICATION=power \
        -DPOWER_ROLE="${role}" \
        -DLORAMAC_LR_FHSS_IS_ON=OFF \
        -DMODULATION=LORA \
        -DREGION_EU868=ON \
        -DBOARD=HeltecLoRa151 \
        -DMBED_RADIO_SHIELD=SX1276MB1LAS

    cmake --build "${build_dir}"
}

case "${1:-all}" in
    send)
        build_role send
        ;;
    receive)
        build_role receive
        ;;
    all)
        build_role send
        build_role receive
        ;;
    *)
        echo "Usage: $0 [send|receive|all]" >&2
        exit 2
        ;;
esac
