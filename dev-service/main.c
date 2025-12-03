#include <stdio.h>

// core includes
#include "cmd_register.h"

// module includes
#include "modules/backlight/backlight.h"

int commands_register()
{
    int ret;

    // register backlight commands
    ret = backlight_cmd_register();

    // add other command registrations here

    return ret;
}

int main(int argc, char *argv[])
{
    // initialize command system
    if (command_init() != 0)
    {
        printf("Failed to initialize command system\n");
        return -1;
    }

    // register commands
    commands_register();

    // print registered commands
    command_print_list();

    return 0;
}