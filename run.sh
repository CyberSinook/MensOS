#!/usr/bin/env bash
#
# MensOS build/run helper.
#
# Usage:
#   ./run.sh build core/safety-runtime
#   ./run.sh run   core/safety-runtime
#   ./run.sh build industry-adapters/robotics
#   ./run.sh run   industry-adapters/robotics
#
# Requires MICROKIT_SDK to be set (already exported inside the
# provided Dockerfile). MICROKIT_BOARD and MICROKIT_CONFIG default
# to qemu_virt_aarch64 / debug and can be overridden as env vars.

set -euo pipefail

ACTION="${1:-}"
TARGET="${2:-}"

MICROKIT_BOARD="${MICROKIT_BOARD:-qemu_virt_aarch64}"
MICROKIT_CONFIG="${MICROKIT_CONFIG:-debug}"

if [ -z "$ACTION" ] || [ -z "$TARGET" ]; then
    echo "Usage: $0 <build|run> <path-to-component>"
    echo "Example: $0 build core/safety-runtime"
    exit 1
fi

if [ -z "${MICROKIT_SDK:-}" ]; then
    echo "Error: MICROKIT_SDK is not set."
    echo "If you're not using the provided Dockerfile, set it manually:"
    echo "  export MICROKIT_SDK=/path/to/microkit-sdk-2.3.0"
    exit 1
fi

if [ ! -d "$TARGET" ]; then
    echo "Error: '$TARGET' is not a directory."
    exit 1
fi

case "$ACTION" in
  build)
    if [[ "$TARGET" == industry-adapters/* ]] && [ ! -d "$TARGET/libvmm-src" ]; then
        echo ">> First-time setup: cloning libvmm into $TARGET/libvmm-src ..."
        git clone --quiet https://github.com/au-ts/libvmm.git "$TARGET/libvmm-src"
        (cd "$TARGET/libvmm-src" && git submodule update --init --recursive --quiet)
    fi

    echo ">> Building $TARGET (board=$MICROKIT_BOARD, config=$MICROKIT_CONFIG) ..."
    mkdir -p "$TARGET/build"
    make -C "$TARGET" \
        BUILD_DIR=build \
        MICROKIT_BOARD="$MICROKIT_BOARD" \
        MICROKIT_CONFIG="$MICROKIT_CONFIG" \
        MICROKIT_SDK="$MICROKIT_SDK"
    echo ">> Build complete: $TARGET/build/loader.img"
    ;;

  run)
    IMAGE="$TARGET/build/loader.img"
    if [ ! -f "$IMAGE" ]; then
        echo "Error: $IMAGE not found. Run '$0 build $TARGET' first."
        exit 1
    fi
    echo ">> Running $TARGET on QEMU AArch64 (Ctrl+A then X to exit) ..."
    qemu-system-aarch64 \
        -machine virt,virtualization=on -cpu cortex-a53 \
        -nographic -serial mon:stdio \
        -device loader,file="$IMAGE",addr=0x70000000,cpu-num=0 \
        -m size=2G
    ;;

  *)
    echo "Unknown action: $ACTION (expected 'build' or 'run')"
    exit 1
    ;;
esac
