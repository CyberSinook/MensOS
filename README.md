# MensOS

**A commercial, multi-industry mixed-criticality operating system built on the [seL4](https://sel4.systems/) microkernel and [seL4 Microkit](https://github.com/seL4/microkit).**

MensOS lets safety-critical software (motor control, steering loops, medical device logic) run natively and provably isolated on a formally verified microkernel, while an untrusted Linux/ROS virtual machine runs alongside it for AI, vision, and general-purpose computing — communicating only through a narrow, auditable IPC boundary.

---

## Why MensOS

Modern safety-critical systems increasingly need two things that have never lived well together in one platform:

- **Provable safety** for the parts that must never fail — motor control, braking, patient-facing medical logic.
- **General-purpose compute** for the parts that benefit from Linux's ecosystem — computer vision, ROS, machine learning.

Closed platforms like QNX and VxWorks offer real-time guarantees but no formal proof of correctness. MensOS is built on **seL4**, the only general-purpose microkernel with a machine-checked proof of functional correctness, and pairs it with **seL4 Microkit**, the officially supported framework for building static, verifiable systems on seL4.

## Architecture

MensOS is deliberately modular so that no product ever ships with capability, attack surface, or certification scope it doesn't need.

```
mensos-dev/
├── core/                        Always required, industry-agnostic
│   ├── safety-runtime/          Native Microkit protection domains (PDs)
│   └── ipc-framework/           Shared message protocol (mensos_ipc.h)
│
├── modules/                     Optional, independently pluggable
│   └── linux-vm/                Isolated Linux VM runtime (built on libvmm)
│       (secure-boot, fleet-management, drivers, and certification
│        packages are planned as additional modules)
│
└── industry-adapters/           Compose only the modules an industry needs
    └── robotics/                First working adapter: Linux VM <-> bridge <-> core
        (automotive, medical, and industrial-plc adapters are planned)
```

### The dependency rule

`core/` never depends on anything in `modules/` — not even the Linux VM. Dependencies only flow one way: `modules/` and `industry-adapters/` may depend on `core/`, never the reverse. This keeps the trusted computing base minimal for products that don't need Linux at all (a simple infusion pump, an industrial safety interlock), and keeps their certification scope small.

### The bridge pattern

Each industry adapter has its own **bridge** protection domain that translates between its Linux VM (if it has one) and `core/`. `core/` and `modules/` never change per industry — only the bridge does. This is the only place industry-specific logic is allowed to live.

### Message flow (robotics adapter, as currently implemented)

```
Linux VM (guest)
   |  keyboard/console activity -> serial IRQ
   v
VMM (vmm_robotics.c, priority 100)
   |  protected call (mensos_ipc message)
   v
robotics_bridge (priority 200)
   |  notify (mensos_ipc message)
   v
safety_runtime (priority 150, in core/)
```

Protected calls in Microkit may only target a **strictly higher priority** protection domain — this is what makes the system statically analyzable and prevents priority-inversion deadlocks. The bridge therefore sits at the highest priority in this chain, since both the VM and the safety-critical PD need to reach it.

## What's working today

| Component | Status |
|---|---|
| `core/safety-runtime` | Two native PDs exchanging heartbeat messages via `mensos_ipc` | Working |
| `core/ipc-framework` | Generic message protocol (`mensos_ipc.h`) | Working |
| `modules/linux-vm` | Independent Linux (Buildroot/BusyBox) VM on seL4/Microkit | Working |
| `industry-adapters/robotics` | Full chain: Linux VM to bridge to safety-critical PD | Working, verified end-to-end on QEMU AArch64 |

This is an early-stage project. The `robotics` adapter currently forwards a placeholder heartbeat message rather than real motor-control commands — it exists to prove the architecture, not as a finished product.

## Getting started

### Prerequisites

MensOS targets **seL4 16.0.0** / **Microkit 2.3.0** on **AArch64** (`qemu_virt_aarch64`). You'll need:

- Docker (the project is developed inside the official [`seL4-CAmkES-L4v-dockerfiles`](https://github.com/seL4/seL4-CAmkES-L4v-dockerfiles) container)
- The [seL4 Microkit SDK](https://github.com/seL4/microkit/releases) (2.3.0)
- `aarch64-linux-gnu-gcc` / `aarch64-linux-gnu-ld` (for native protection domains)
- `clang`, `lld`, `llvm`, `dtc` (for the Linux VM / VMM, via [libvmm](https://github.com/au-ts/libvmm))
- `qemu-system-aarch64`

If `ld.lld` isn't found even though `lld` is installed, it's often only present as a versioned binary:

```sh
sudo ln -sf /usr/bin/ld.lld-19 /usr/bin/ld.lld
```

### Building `core/safety-runtime` (no Linux VM required)

```sh
cd core/safety-runtime
mkdir -p build
make BUILD_DIR=build \
     MICROKIT_BOARD=qemu_virt_aarch64 \
     MICROKIT_CONFIG=debug \
     MICROKIT_SDK=/path/to/microkit-sdk-2.3.0
```

Run it:

```sh
qemu-system-aarch64 \
  -machine virt,virtualization=on -cpu cortex-a53 \
  -nographic -serial mon:stdio \
  -device loader,file=build/loader.img,addr=0x70000000,cpu-num=0 \
  -m size=2G
```

You should see the safety-critical PD send a heartbeat and the monitor PD receive it.

### Building `industry-adapters/robotics` (Linux VM + bridge + core)

```sh
cd industry-adapters/robotics
git clone https://github.com/au-ts/libvmm.git libvmm-src
cd libvmm-src && git submodule update --init --recursive && cd ..
make MICROKIT_SDK=/path/to/microkit-sdk-2.3.0
```

The first build downloads a prebuilt Linux kernel and initrd (Buildroot/BusyBox) automatically. Run it:

```sh
qemu-system-aarch64 \
  -machine virt,virtualization=on -cpu cortex-a53 \
  -nographic -serial mon:stdio \
  -device loader,file=build/loader.img,addr=0x70000000,cpu-num=0 \
  -m size=2G
```

Log in as `root` (no password) and type anything — each keystroke triggers a serial interrupt that flows through the VMM, the bridge, and into the safety-critical PD, which will print that it received a heartbeat.

Exit QEMU with `Ctrl+A` then `X`.

## Business model

MensOS follows an **open-core** model. The seL4 kernel itself is, and will always be, free and open — it's governed independently by the [seL4 Foundation](https://sel4.systems/Foundation/). What MensOS sells sits on top of it: hardware drivers, certification documentation packages (ISO 26262, IEC 62304, IEC 61508), secure boot and key management, fleet management, and support.

## Roadmap

- **Year 1** — Technical foundation (current phase)
- **Year 2** — MVP and first robotics pilot
- **Year 3** — Revenue and credibility to expand into automotive

Target market order: robotics first (fastest sales cycle), in parallel with confidential computing, then automotive/software-defined vehicles (the largest opportunity), then medical, then industrial infrastructure.

## License

Licensing terms for MensOS source code are still being finalized. seL4, Microkit, and libvmm retain their own upstream licenses (BSD-2-Clause / Apache-2.0 / CC-BY-SA-4.0 as applicable) and are not affected by this repository's license.

## Acknowledgements

MensOS builds directly on the work of the [seL4 Foundation](https://sel4.systems/), the [Trustworthy Systems](https://trustworthy.systems/) group at UNSW, and the [libvmm](https://github.com/au-ts/libvmm) project.
