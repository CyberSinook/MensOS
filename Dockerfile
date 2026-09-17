# MensOS Development Environment
#
# Builds a self-contained container with everything needed to build
# core/, modules/, and industry-adapters/ components for MensOS on
# qemu_virt_aarch64, without any manual setup steps.
#
# Usage:
#   docker build -t mensos-dev .
#   docker run -it --rm -v $(pwd):/workspace mensos-dev
#
# Inside the container, MICROKIT_SDK is already set and exported.

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# --- Base tools ---
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    git \
    curl \
    wget \
    ca-certificates \
    make \
    python3 \
    device-tree-compiler \
    qemu-system-arm \
    ipxe-qemu \
    gcc-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    clang \
    lld \
    llvm \
    && rm -rf /var/lib/apt/lists/*

# --- Fix versioned lld binary (common on Ubuntu/Debian packaging) ---
RUN LLD_BIN=$(ls /usr/bin/ld.lld-* 2>/dev/null | head -n1) && \
    if [ -n "$LLD_BIN" ] && [ ! -e /usr/bin/ld.lld ]; then \
        ln -sf "$LLD_BIN" /usr/bin/ld.lld; \
    fi

# --- seL4 Microkit SDK ---
ARG MICROKIT_VERSION=2.3.0
ENV MICROKIT_SDK=/opt/microkit-sdk-${MICROKIT_VERSION}

RUN curl -L \
    "https://github.com/seL4/microkit/releases/download/${MICROKIT_VERSION}/microkit-sdk-${MICROKIT_VERSION}-linux-x86-64.tar.gz" \
    -o /tmp/microkit-sdk.tar.gz && \
    mkdir -p /opt && \
    tar xf /tmp/microkit-sdk.tar.gz -C /opt && \
    rm /tmp/microkit-sdk.tar.gz

# --- Default build parameters, override as needed ---
ENV MICROKIT_BOARD=qemu_virt_aarch64
ENV MICROKIT_CONFIG=debug

WORKDIR /workspace

CMD ["/bin/bash"]
