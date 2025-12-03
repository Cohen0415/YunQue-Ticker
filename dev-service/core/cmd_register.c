#include <stdio.h>
#include <string.h>

#include "cmd_register.h"

int register_command(const char *cmd_name, int (*handler)(char *value))
{
    if (cmd_name == NULL || handler == NULL) 
    {
        return -1; // Return error if command name or handler is NULL
    }

    printf("Registered command: %s\n", cmd_name);

    return 0; // Return 0 on success
}