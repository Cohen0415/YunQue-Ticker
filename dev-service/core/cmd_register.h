#ifndef CMD_REGISTER_H
#define CMD_REGISTER_H

typedef struct command {
    const char *name;
    int (*handler)(char *value);
} command_t;

int command_register(command_t *cmd);
void command_free_all(void);
int command_init(void);
void command_print_list(void);

#endif // CMD_REGISTER_H