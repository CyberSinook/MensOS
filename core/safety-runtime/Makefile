ifeq ($(strip $(BUILD_DIR)),)
$(error BUILD_DIR must be specified)
endif
ifeq ($(strip $(MICROKIT_SDK)),)
$(error MICROKIT_SDK must be specified)
endif
ifeq ($(strip $(MICROKIT_BOARD)),)
$(error MICROKIT_BOARD must be specified)
endif
ifeq ($(strip $(MICROKIT_CONFIG)),)
$(error MICROKIT_CONFIG must be specified)
endif

BOARD_DIR := $(MICROKIT_SDK)/board/$(MICROKIT_BOARD)/$(MICROKIT_CONFIG)

CC := aarch64-linux-gnu-gcc
LD := aarch64-linux-gnu-ld
AS := aarch64-linux-gnu-as

MICROKIT_TOOL ?= $(MICROKIT_SDK)/bin/microkit

CFLAGS := -nostdlib -ffreestanding -g -O3 -Wall -Wno-unused-function -Werror -I$(BOARD_DIR)/include -I../ipc-framework/include -mstrict-align
LDFLAGS := -L$(BOARD_DIR)/lib
LIBS := -lmicrokit -Tmicrokit.ld

IMAGE_FILE = $(BUILD_DIR)/loader.img
REPORT_FILE = $(BUILD_DIR)/report.txt

all: $(IMAGE_FILE)

$(BUILD_DIR)/%.o: %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/safety_runtime.elf: $(BUILD_DIR)/safety_runtime.o
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/heartbeat_monitor.elf: $(BUILD_DIR)/heartbeat_monitor.o
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(IMAGE_FILE) $(REPORT_FILE): $(BUILD_DIR)/safety_runtime.elf $(BUILD_DIR)/heartbeat_monitor.elf safety_runtime.system
	$(MICROKIT_TOOL) safety_runtime.system --search-path $(BUILD_DIR) --board $(MICROKIT_BOARD) --config $(MICROKIT_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)
