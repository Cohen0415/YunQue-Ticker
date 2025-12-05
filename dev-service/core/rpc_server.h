#ifndef RPC_SERVER_H
#define RPC_SERVER_H

typedef int (*rpc_on_message_cb)(const char *request, int request_len, char **response);

int rpc_server_init_uds(const char *path);
void rpc_server_run(rpc_on_message_cb cb);
void rpc_server_stop(void);
int rpc_make_response(int status, const char *msg, const char *data_json, char **resp);
int rpc_make_error(char **resp, int status, const char *msg);

#endif // RPC_SERVER_H