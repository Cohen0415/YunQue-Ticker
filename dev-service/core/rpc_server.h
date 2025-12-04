#ifndef RPC_SERVER_H
#define RPC_SERVER_H

typedef void (*rpc_on_message_cb)(int client_fd, const char *data, int len);

int rpc_server_init_uds(const char *path);
void rpc_server_run(rpc_on_message_cb cb);
void rpc_server_stop(void);
int rpc_server_send_response(int client_fd, const char *data);

#endif // RPC_SERVER_H