#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

// core includes
#include "cmd_register.h"
#include "rpc_server.h"

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

void on_msg(int client_fd, char **data, int len)
{
    LOGD("recv(%d bytes): %.*s\n", len, len, *data);

    // change data to "ack"
    *data = malloc(4);
    snprintf(*data, 4, "ack"); 
    len = 3;
}

int main(int argc, char *argv[])
{
    // initialize logging system
    if (log_init("dev-service.log") != 0)
    {
        printf("Failed to initialize logging system\n");
        return -1;
    }
    LOGI("Logging system initialized\n");
    
    // initialize command system
    if (command_init() != 0)
    {
        printf("Failed to initialize command system\n");
        return -1;
    }
    LOGI("Command system initialized\n");

    // initialize uds rpc server
    if (rpc_server_init_uds("/tmp/dev.sock") != 0)
    {
        printf("Failed to initialize RPC server\n");
        return -1;
    }
    LOGI("RPC server initialized\n");

    // register commands
    commands_register();

    // run rpc server
    rpc_server_run(on_msg);

    // log close
    log_close();

    return 0;
}