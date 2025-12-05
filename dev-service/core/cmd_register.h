#ifndef CMD_REGISTER_H
#define CMD_REGISTER_H

#include "cJSON.h"

typedef struct command {
    const char *name;
    int (*handler)(cJSON *params, char **data_json);
} command_t;

int command_register(command_t *cmd);
void command_free_all(void);
int command_init(void);
void command_print_list(void);
command_t *command_find(const char *name);

#endif // CMD_REGISTER_H