#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "cJSON.h"

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

void on_msg(int client_fd, const char *request, char **response, int len)
{
    LOGD("recv(%d bytes): %s", len, request);

    /* parse JSON */
    cJSON *root = cJSON_Parse(request);
    if (!root)
    {
        LOGE("JSON parse failed");
        return;
    }

    /* get cmd string */
    cJSON *cmd_json = cJSON_GetObjectItem(root, "cmd");
    if (!cJSON_IsString(cmd_json))
    {
        LOGE("Invalid or missing cmd field");
        cJSON_Delete(root);
        return;
    }

    const char *cmd_name = cmd_json->valuestring;

    /* find the cmd */
    command_t *cmd = command_find(cmd_name);
    if (cmd)
        LOGI("Found command: %s", cmd->name);
    else
        LOGW("Command not registered: %s", cmd_name);

    /* execute modules function */
    int ret = -1;
    if (cmd && cmd->handler)
    {
        ret = cmd->handler(request, response);
        LOGI("Command %s executed with return code: %d", cmd_name, ret);
    }
    else
    {
        LOGW("No handler for command: %s", cmd_name);
    }

    cJSON_Delete(root);
}

int main(int argc, char *argv[])
{
    // initialize logging system
    if (log_init("dev-service.log") != 0)
    {
        printf("Failed to initialize logging system\n");
        return -1;
    }
    LOGI("Logging system initialized");
    
    // initialize command system
    if (command_init() != 0)
    {
        printf("Failed to initialize command system\n");
        return -1;
    }
    LOGI("Command system initialized");

    // initialize uds rpc server
    if (rpc_server_init_uds("/tmp/dev.sock") != 0)
    {
        printf("Failed to initialize RPC server\n");
        return -1;
    }
    LOGI("RPC server initialized");

    // register commands
    commands_register();

    // run rpc server
    rpc_server_run(on_msg);

    // log close
    log_close();

    return 0;
}