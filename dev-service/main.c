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

void on_msg(int client_fd, const char *data, int len)
{
    printf("recv(%d bytes): %.*s\n", len, len, data);

    // 先做简单回显
    rpc_server_send_response(client_fd, "OK");
}

int main(int argc, char *argv[])
{
    // initialize command system
    if (command_init() != 0)
    {
        printf("Failed to initialize command system\n");
        return -1;
    }

    // initialize uds rpc server
    if (rpc_server_init_uds("/tmp/dev.sock") != 0)
    {
        printf("Failed to initialize RPC server\n");
        return -1;
    }

    // register commands
    commands_register();

    // print registered commands
    command_print_list();

    // run rpc server
    rpc_server_run(on_msg);

    return 0;
}