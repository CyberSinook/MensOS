ifeq ($(strip $(MICROKIT_SDK)),)
$(error MICROKIT_SDK must be specified)
endif
MICROKIT_BOARD := qemu_virt_aarch64
MICROKIT_CONFIG := debug
BOARD_DIR := $(MICROKIT_SDK)/board/$(MICROKIT_BOARD)/$(MICROKIT_CONFIG)
MICROKIT_TOOL := $(MICROKIT_SDK)/bin/microkit

BUILD_DIR := build
LIBVMM := $(abspath libvmm-src)
SDDF := $(abspath libvmm-src/dep/sddf)

CC_NATIVE := aarch64-linux-gnu-gcc
LD_NATIVE := aarch64-linux-gnu-ld
CFLAGS_NATIVE := -nostdlib -ffreestanding -g -O3 -Wall -Wno-unused-function -Werror \
                 -I$(BOARD_DIR)/include -I../../core/ipc-framework/include -mstrict-align
LDFLAGS_NATIVE := -L$(BOARD_DIR)/lib
LIBS_NATIVE := -lmicrokit -Tmicrokit.ld

CC := clang
LD := ld.lld
AS := llvm-as
AR := llvm-ar
OBJCOPY := llvm-objcopy
DTC := dtc
RANLIB := llvm-ranlib

export MICROKIT_CONFIG
export BUILD_DIR := $(abspath $(BUILD_DIR))
export EXAMPLE_DIR := $(abspath .)
export CC
export LD
export AS
export AR
export OBJCOPY
export DTC
export RANLIB
export MICROKIT_TOOL
export SDDF
export LIBVMM
export ARCH := aarch64

CFLAGS_VMM := -ffreestanding -g3 -O3 -Wall -Wno-unused-function \
              -DBOARD_$(MICROKIT_BOARD) \
              -I$(BOARD_DIR)/include -I$(LIBVMM)/include \
              -I$(SDDF)/include -I$(SDDF)/include/sddf/util/custom_libc -I$(SDDF)/include/microkit \
              -I../../core/ipc-framework/include \
              -MD -MP -target aarch64-none-elf -mstrict-align
LDFLAGS_VMM := -L$(BOARD_DIR)/lib
LIBS_VMM := --start-group -lmicrokit -Tmicrokit.ld $(BUILD_DIR)/libvmm.a $(BUILD_DIR)/libsddf_util_debug.a --end-group

LINUX := 8b1d3a8587c60428c79d3e1981e7b6a7c653e1f8-linux
INITRD := e800d52939b73281670df8598cb9c616dfd44f10-rootfs.cpio.gz

vpath %.c $(LIBVMM) $(EXAMPLE_DIR)

IMAGE_FILE := $(BUILD_DIR)/loader.img
REPORT_FILE := $(BUILD_DIR)/report.txt
SYSTEM_FILE := robotics.system

all: $(IMAGE_FILE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/safety_runtime.o: ../../core/safety-runtime/safety_runtime.c | $(BUILD_DIR)
	$(CC_NATIVE) -c $(CFLAGS_NATIVE) $< -o $@

$(BUILD_DIR)/safety_runtime.elf: $(BUILD_DIR)/safety_runtime.o
	$(LD_NATIVE) $(LDFLAGS_NATIVE) $^ $(LIBS_NATIVE) -o $@

$(BUILD_DIR)/robotics_bridge.o: bridge/robotics_bridge.c | $(BUILD_DIR)
	$(CC_NATIVE) -c $(CFLAGS_NATIVE) $< -o $@

$(BUILD_DIR)/robotics_bridge.elf: $(BUILD_DIR)/robotics_bridge.o
	$(LD_NATIVE) $(LDFLAGS_NATIVE) $^ $(LIBS_NATIVE) -o $@

${LINUX}:
	curl -L https://trustworthy.systems/Downloads/libvmm/images/${LINUX}.tar.gz -o $@.tar.gz
	mkdir -p linux_download_dir
	tar -xf $@.tar.gz -C linux_download_dir
	cp linux_download_dir/${LINUX}/Image ${LINUX}

${INITRD}:
	curl -L https://trustworthy.systems/Downloads/libvmm/images/${INITRD}.tar.gz -o $@.tar.gz
	mkdir -p initrd_download_dir
	tar xf $@.tar.gz -C initrd_download_dir
	cp initrd_download_dir/${INITRD}/rootfs.cpio.gz ${INITRD}

vm.dts: libvmm-src/examples/simple/board/qemu_virt_aarch64/linux.dts libvmm-src/examples/simple/board/qemu_virt_aarch64/overlay.dts
	$(LIBVMM)/tools/dtscat $^ > $@

vm.dtb: vm.dts
	$(DTC) -q -I dts -O dtb $< > $@

$(BUILD_DIR)/vmm_robotics.o: vmm_robotics.c | $(BUILD_DIR)
	$(CC) $(CFLAGS_VMM) -c -o $@ $<

$(BUILD_DIR)/images.o: $(LIBVMM)/tools/package_guest_images.S ${LINUX} ${INITRD} vm.dtb | $(BUILD_DIR)
	$(CC) -c -g3 -x assembler-with-cpp \
		-DGUEST_KERNEL_IMAGE_PATH=\"$(abspath ${LINUX})\" \
		-DGUEST_DTB_IMAGE_PATH=\"$(abspath vm.dtb)\" \
		-DGUEST_INITRD_IMAGE_PATH=\"$(abspath ${INITRD})\" \
		-target aarch64-none-elf -mstrict-align \
		$(LIBVMM)/tools/package_guest_images.S -o $@

$(BUILD_DIR)/vmm_robotics.elf: $(BUILD_DIR)/vmm_robotics.o $(BUILD_DIR)/images.o $(BUILD_DIR)/libvmm.a $(BUILD_DIR)/libsddf_util_debug.a
	$(LD) $(LDFLAGS_VMM) $(BUILD_DIR)/vmm_robotics.o $(BUILD_DIR)/images.o $(LIBS_VMM) -o $@

$(BUILD_DIR)/libvmm.a $(BUILD_DIR)/libsddf_util_debug.a:
	mkdir -p $(BUILD_DIR)/util $(BUILD_DIR)/util/custom_libc
	mkdir -p $(BUILD_DIR)/libvmm/arch/aarch64/vgic $(BUILD_DIR)/libvmm/util $(BUILD_DIR)/libvmm/virtio $(BUILD_DIR)/libvmm/uefi
	$(MAKE) -C $(BUILD_DIR) -f $(LIBVMM)/vmm.mk MICROKIT_SDK=$(MICROKIT_SDK) MICROKIT_CONFIG=$(MICROKIT_CONFIG) VPATH=$(LIBVMM) ARCH=aarch64 SDDF=$(SDDF) LIBVMM=$(LIBVMM) CFLAGS="-I$(BOARD_DIR)/include -I$(SDDF)/include -I$(SDDF)/include/microkit -I$(SDDF)/include/sddf/util/custom_libc -target aarch64-none-elf -mstrict-align" libvmm.a
	$(MAKE) -C $(BUILD_DIR) -f $(SDDF)/util/util.mk MICROKIT_SDK=$(MICROKIT_SDK) VPATH=$(SDDF) ARCH=aarch64 SDDF=$(SDDF) SDDF_CUSTOM_LIBC=1 CFLAGS="-I$(BOARD_DIR)/include -I$(SDDF)/include -I$(SDDF)/include/microkit -I$(SDDF)/include/sddf/util/custom_libc -target aarch64-none-elf -mstrict-align" libsddf_util_debug.a

$(IMAGE_FILE) $(REPORT_FILE): $(BUILD_DIR)/safety_runtime.elf $(BUILD_DIR)/robotics_bridge.elf $(BUILD_DIR)/vmm_robotics.elf $(SYSTEM_FILE)
	$(MICROKIT_TOOL) $(SYSTEM_FILE) --search-path $(BUILD_DIR) --board $(MICROKIT_BOARD) --config $(MICROKIT_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)

qemu: $(IMAGE_FILE)
	qemu-system-aarch64 -machine virt,virtualization=on -cpu cortex-a53 \
		-device loader,file=$(IMAGE_FILE),addr=0x70000000,cpu-num=0 \
		-serial mon:stdio -m size=2G -nographic
