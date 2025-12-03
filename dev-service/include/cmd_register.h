#ifndef CMD_REGISTER_H
#define CMD_REGISTER_H

int register_command(const char *cmd_name, int (*handler)(char *value));

#endif // CMD_REGISTER_H