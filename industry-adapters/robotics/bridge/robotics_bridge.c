#include <microkit.h>
#include "mensos_ipc.h"

#define VM_CH             0
#define SAFETY_RUNTIME_CH 1

void init(void)
{
    microkit_dbg_puts("ROBOTICS_BRIDGE|INFO: ready, waiting for VM messages\n");
}

microkit_msginfo protected(microkit_channel ch, microkit_msginfo msginfo)
{
    switch (ch) {
    case VM_CH: {
        mensos_cmd_type_t cmd = mensos_msg_get_cmd_type();
        uint64_t data = mensos_msg_get_data0();
        microkit_dbg_puts("ROBOTICS_BRIDGE|INFO: received message from VM\n");

        (void) mensos_msg_new(cmd, data);
        microkit_notify(SAFETY_RUNTIME_CH);
        break;
    }
    default:
        microkit_dbg_puts("ROBOTICS_BRIDGE|ERROR: unexpected channel\n");
    }
    return microkit_msginfo_new(0, 0);
}

void notified(microkit_channel ch)
{
    microkit_dbg_puts("ROBOTICS_BRIDGE|ERROR: unexpected notification\n");
}
