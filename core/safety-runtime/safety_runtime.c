#include <microkit.h>
#include "mensos_ipc.h"

#define HEARTBEAT_MONITOR_CH 0

void init(void)
{
    microkit_dbg_puts("SAFETY_RUNTIME|INFO: starting up, beginning heartbeat loop\n");
    (void) mensos_msg_new(MENSOS_CMD_HEARTBEAT, 1);
    microkit_notify(HEARTBEAT_MONITOR_CH);
}

void notified(microkit_channel ch)
{
    mensos_cmd_type_t cmd = mensos_msg_get_cmd_type();
    switch (cmd) {
    case MENSOS_CMD_HEARTBEAT:
        microkit_dbg_puts("SAFETY_RUNTIME|HEARTBEAT: received on channel\n");
        break;
    case MENSOS_CMD_STATUS:
        microkit_dbg_puts("SAFETY_RUNTIME|STATUS: received status message\n");
        break;
    default:
        microkit_dbg_puts("SAFETY_RUNTIME|INFO: received unrecognized message type\n");
    }
}
