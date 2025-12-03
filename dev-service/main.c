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
    // register commands
    commands_register();

    return 0;
}