#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <microkit.h>
#include <libvmm/libvmm.h>
#include "mensos_ipc.h"

#define GUEST_RAM_SIZE 0x10000000

#if defined(BOARD_qemu_virt_aarch64)
#define GUEST_RAM_START_GPA 0x40000000
#define GUEST_DTB_GPA 0x4f000000
#define GUEST_INIT_RAM_DISK_GPA 0x4d000000
#else
#endif

#define SERIAL_IRQ_CH 1
#define SERIAL_IRQ 33

#define BRIDGE_CH 2

extern char _guest_kernel_image[];
extern char _guest_kernel_image_end[];
extern char _guest_initrd_image[];
extern char _guest_initrd_image_end[];
extern char _guest_dtb_image[];
extern char _guest_dtb_image_end[];

uintptr_t guest_ram_vaddr;

void init(void)
{
    LOG_VMM("starting \"%s\"\n", microkit_name);

    arch_guest_init_t args = {
        .pci_init.mmio_aperature_size = 0,
        .num_vcpus = 1,
        .num_guest_ram_regions = 1,
        .guest_ram_regions = { (struct guest_ram_region) {
            .gpa_start = GUEST_RAM_START_GPA, .size = GUEST_RAM_SIZE, .vmm_vaddr = (void *)guest_ram_vaddr } }
    };
    bool success = guest_init(args);
    if (!success) {
        LOG_VMM_ERR("Failed to initialise guest\n");
        return;
    }

    size_t kernel_size = _guest_kernel_image_end - _guest_kernel_image;
    size_t initrd_size = _guest_initrd_image_end - _guest_initrd_image;
    size_t dtb_size = _guest_dtb_image_end - _guest_dtb_image;

    if (!kernel_size || !initrd_size || !dtb_size) {
        LOG_VMM_ERR("Guest image(s) empty\n");
        return;
    }

    uintptr_t kernel_pc = linux_setup_images(GUEST_RAM_START_GPA, (uintptr_t)_guest_kernel_image, kernel_size,
                                             (uintptr_t)_guest_dtb_image, GUEST_DTB_GPA, dtb_size,
                                             (uintptr_t)_guest_initrd_image, GUEST_INIT_RAM_DISK_GPA, initrd_size);
    if (!kernel_pc) {
        LOG_VMM_ERR("Failed to initialise guest images\n");
        return;
    }

    success = virq_register_passthrough(ARM_GIC_IRQ_ROUTE(GUEST_BOOT_VCPU_ID, SERIAL_IRQ), SERIAL_IRQ_CH);
    assert(success);

    (void) mensos_msg_new(MENSOS_CMD_STATUS, 1);
    microkit_msginfo info = mensos_msg_new(MENSOS_CMD_STATUS, 1);
    microkit_ppcall(BRIDGE_CH, info);

    guest_start(kernel_pc, GUEST_DTB_GPA, GUEST_INIT_RAM_DISK_GPA);
}

void notified(microkit_channel ch)
{
    switch (ch) {
    case SERIAL_IRQ_CH: {
        bool success = virq_handle_passthrough(ch);
        if (!success) {
            LOG_VMM_ERR("Serial IRQ dropped\n");
        }

        microkit_msginfo info = mensos_msg_new(MENSOS_CMD_HEARTBEAT, 1);
        microkit_ppcall(BRIDGE_CH, info);
        break;
    }
    default:
        printf("Unexpected channel, ch: 0x%x\n", ch);
    }
}

seL4_Bool fault(microkit_child child, microkit_msginfo msginfo, microkit_msginfo *reply_msginfo)
{
    bool success = fault_handle(child, msginfo);
    if (success) {
        *reply_msginfo = microkit_msginfo_new(0, 0);
        return seL4_True;
    }
    return seL4_False;
}
