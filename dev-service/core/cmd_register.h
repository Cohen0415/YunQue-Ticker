#ifndef CMD_REGISTER_H
#define CMD_REGISTER_H

#include "cJSON.h"
#include "rpc_server.h"

typedef struct command {
    const char *name;
    rpc_result_t (*handler)(cJSON *params);
} command_t;

int command_register(command_t *cmd);
void command_free_all(void);
int command_init(void);
void command_print_list(void);
command_t *command_find(const char *name);

#endif // CMD_REGISTER_H