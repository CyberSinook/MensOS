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
    microkit_dbg_puts("SAFETY_RUNTIME|ERROR: unexpected notification\n");
}
