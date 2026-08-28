#include <microkit.h>
#include "mensos_ipc.h"

#define SAFETY_RUNTIME_CH 0

void init(void)
{
    microkit_dbg_puts("MONITOR|INFO: waiting for heartbeats\n");
}

void notified(microkit_channel ch)
{
    switch (ch) {
    case SAFETY_RUNTIME_CH:
        microkit_dbg_puts("MONITOR|HEARTBEAT: received tick via mensos_ipc protocol\n");
        break;
    default:
        microkit_dbg_puts("MONITOR|ERROR: unexpected channel\n");
    }
}
