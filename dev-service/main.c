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
#include "modules/sysinfo/sysinfo.h"

int commands_register()
{
    int ret;

    // register backlight commands
    ret = backlight_cmd_register();
    
    // register sysinfo commands
    ret = sysinfo_cmd_register();

    // add other command registrations here

    return ret;
}

int rpc_on_msg(const char *request, int request_len, char **response)
{
    LOGD("recv(%d bytes): %s", request_len, request);

    /* parse JSON */
    cJSON *root = cJSON_Parse(request);
    if (!root)
    {
        LOGE("JSON parse failed");
        return rpc_make_error(response, -1, "JSON parse error");
    }

    cJSON *cmd = cJSON_GetObjectItem(root, "cmd");
    cJSON *params = cJSON_GetObjectItem(root, "params");

    if (!cJSON_IsString(cmd))
    {
        LOGE("Invalid or missing cmd field");
        cJSON_Delete(root);
        return rpc_make_error(response, -2, "cmd missing or invalid");
    }

    command_t *c = command_find(cmd->valuestring);
    if (!c) 
    {
        cJSON_Delete(root);
        return rpc_make_error(response, -3, "unknown command");
    }
    LOGI("Found command: %s", c->name);

    char *data_json = NULL;
    int ret = c->handler(params, &data_json);

    int status = (ret == 0 ? 0 : ret);
    rpc_make_response(status, status==0?"ok":"fail", data_json, response);

    free(data_json);
    cJSON_Delete(root);
    return status;
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
    rpc_server_run(rpc_on_msg);

    // log close
    log_close();

    return 0;
}