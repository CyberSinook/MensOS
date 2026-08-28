#ifndef MENSOS_IPC_H
#define MENSOS_IPC_H

#include <microkit.h>
#include <stdint.h>

typedef enum {
    MENSOS_CMD_HEARTBEAT   = 0,
    MENSOS_CMD_DATA        = 1,
    MENSOS_CMD_COMMAND     = 2,
    MENSOS_CMD_STATUS      = 3,
    MENSOS_CMD_EMERGENCY   = 4
} mensos_cmd_type_t;

#define MENSOS_MR_CMD_TYPE   0
#define MENSOS_MR_DATA_0     1
#define MENSOS_MR_DATA_1     2
#define MENSOS_MR_TIMESTAMP  3

static inline microkit_msginfo mensos_msg_new(mensos_cmd_type_t cmd_type, uint64_t data0)
{
    microkit_mr_set(MENSOS_MR_CMD_TYPE, (uint64_t)cmd_type);
    microkit_mr_set(MENSOS_MR_DATA_0, data0);
    return microkit_msginfo_new(0, 2);
}

static inline mensos_cmd_type_t mensos_msg_get_cmd_type(void)
{
    return (mensos_cmd_type_t)microkit_mr_get(MENSOS_MR_CMD_TYPE);
}

static inline uint64_t mensos_msg_get_data0(void)
{
    return microkit_mr_get(MENSOS_MR_DATA_0);
}

#endif
