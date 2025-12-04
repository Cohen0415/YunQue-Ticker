#include "rpc_server.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static int g_listen_fd = -1;
static int g_running = 0;

static char *read_all(int fd, int *out_len)
{
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if(!buf) 
        return NULL;

    for(;;)
    {
        ssize_t n = read(fd, buf+len, cap-len);
        if(n > 0)
        {
            len += n;
            if(len >= cap)
            {
                cap *= 2;
                char *tmp = realloc(buf, cap);
                if(!tmp)
                { 
                    free(buf); 
                    return NULL; 
                }
                buf = tmp;
            }
        }
        else if(n == 0) 
            break;       // 读完
        else if(errno == EINTR) 
            continue;
        else 
        { 
            free(buf); 
            return NULL; 
        }
    }

    *out_len = len;
    return buf;
}

int rpc_server_init_uds(const char *path)
{
    unlink(path);
    g_listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(g_listen_fd < 0) 
        return -1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

    if(bind(g_listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
        return -1;
    if(listen(g_listen_fd, 8) < 0) 
        return -1;

    g_running = 1;
    return 0;
}

void rpc_server_run(rpc_on_message_cb cb)
{
    if (g_listen_fd < 0 || !cb)
        return;

    while(g_running)
    {
        int client_fd = accept(g_listen_fd, NULL, NULL);
        if(client_fd < 0) 
            continue;

        int len = 0;
        char *data = read_all(client_fd, &len);
        if(data && len > 0)
        {
            cb(client_fd, data, len); // 重点：传回 client_fd
            free(data);
        }

        close(client_fd);
    }
}

void rpc_server_stop(void)
{
    g_running = 0;
    if(g_listen_fd >= 0)
    {
        close(g_listen_fd);
        g_listen_fd = -1;
    }
}

int rpc_server_send_response(int client_fd, const char *data)
{
    if (!data) return -1;

    int sent = write(client_fd, data, strlen(data));
    return sent;
}