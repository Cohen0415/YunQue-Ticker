#include "rpc_server.h"
#include "log.h"
#include "queue.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

static int g_listen_fd = -1;
static int g_running = 0;

static queue_t *recv_queue = NULL;
static queue_t *send_queue = NULL;

typedef struct {
    int client_fd;
    char *data;
} rpc_msg_t;

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
            break;       
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

// 接收线程
static void *recv_thread(void *arg)
{
    rpc_on_message_cb cb = (rpc_on_message_cb)arg;

    while(g_running)
    {
        int client_fd = accept(g_listen_fd, NULL, NULL);
        if(client_fd < 0) continue;

        int len = 0;
        char *data = read_all(client_fd, &len);
        if(data && len > 0)
        {
            rpc_msg_t *msg = malloc(sizeof(rpc_msg_t));
            msg->client_fd = client_fd;
            msg->data = data;
            queue_push(recv_queue, msg); // 放到接收队列
        } 
        else 
        {
            close(client_fd);
        }
    }

    return NULL;
}

// 处理线程
static void *process_thread(void *arg)
{
    rpc_on_message_cb cb = (rpc_on_message_cb)arg;

    while(g_running)
    {
        rpc_msg_t *msg = (rpc_msg_t*)queue_pop(recv_queue);
        if(!msg) 
            continue;

        // 执行回调
        cb(msg->client_fd, &(msg->data), strlen(msg->data));

        // 处理完的数据直接入发送队列（这里可包装成响应）
        queue_push(send_queue, msg); 
        // 注意：发送后释放 msg 以及 msg->data 在发送线程完成
    }

    return NULL;
}

// 发送线程
static void *send_thread(void *arg)
{
    (void)arg;

    while(g_running)
    {
        rpc_msg_t *msg = (rpc_msg_t*)queue_pop(send_queue);
        if(!msg) 
            continue;

        LOGD("send response to client_fd=%d: %s\n", msg->client_fd, msg->data);
        write(msg->client_fd, msg->data, strlen(msg->data));
        close(msg->client_fd);
        free(msg->data);
        free(msg);
    }

    return NULL;
}

int rpc_server_init_uds(const char *path)
{
    unlink(path);
    g_listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(g_listen_fd < 0) 
    {
        LOGE("socket error: %s\n", strerror(errno));
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

    if(bind(g_listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
    {
        LOGE("bind error: %s\n", strerror(errno));
        return -1;
    }
    if(listen(g_listen_fd, 8) < 0) 
    {
        LOGE("listen error: %s\n", strerror(errno));
        return -1;
    }

    recv_queue = queue_create();
    send_queue = queue_create();

    g_running = 1;
    return 0;
}

void rpc_server_run(rpc_on_message_cb cb)
{
    pthread_t tid_recv, tid_process, tid_send;

    pthread_create(&tid_recv, NULL, recv_thread, cb);
    pthread_create(&tid_process, NULL, process_thread, cb);
    pthread_create(&tid_send, NULL, send_thread, NULL);

    pthread_join(tid_recv, NULL);
    pthread_join(tid_process, NULL);
    pthread_join(tid_send, NULL);
}

void rpc_server_stop(void)
{
    g_running = 0;
    if(g_listen_fd >= 0)
    {
        close(g_listen_fd);
        g_listen_fd = -1;
    }

    if(recv_queue) 
        queue_destroy(recv_queue);
    if(send_queue) 
        queue_destroy(send_queue);
}