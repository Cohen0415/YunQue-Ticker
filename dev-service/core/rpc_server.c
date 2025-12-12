#include "rpc_server.h"
#include "log.h"
#include "queue.h"
#include "cJSON.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <endian.h>

static int g_listen_fd = -1;
static int g_running = 0;

static queue_t *recv_queue = NULL;
static queue_t *send_queue = NULL;

typedef struct {
    int client_fd;
    char *request;
    char *response;
} rpc_msg_t;

static ssize_t read_fully(int fd, void *buf, size_t count) 
{
    char *p = buf;
    size_t total = 0;
    while (total < count) 
    {
        ssize_t n = read(fd, p + total, count - total);
        if (n <= 0) 
        {
            if (n == 0 || errno == ECONNRESET || errno == EPIPE) 
            {
                return 0; // 对端关闭或重置
            }
            if (errno == EINTR) 
                continue;
            return -1; // 错误
        }
        total += n;
    }

    return total;
}

static void handle_client_connection(int client_fd) 
{
    while (g_running) 
    {
        // Step 1: 读取 4 字节长度头
        unsigned int net_len;
        ssize_t r = read_fully(client_fd, &net_len, sizeof(net_len));
        if (r != sizeof(net_len)) 
        {
            break;
        }

        // 转为主机字节序（小端 → 主机）
        unsigned int len = le32toh(net_len);
        if (len == 0 || len > MAX_RPC_MSG_SIZE) 
        {
            LOGE("Invalid message length: %u", len);
            break;
        }

        // Step 2: 读取 len 字节的 JSON 数据
        char *json_str = malloc(len + 1);
        if (!json_str) 
        {
            LOGE("malloc failed for message of size %u", len);
            break;
        }

        r = read_fully(client_fd, json_str, len);
        if (r != len) 
        {
            free(json_str);
            break;
        }
        json_str[len] = '\0'; // 确保字符串结尾

        // 构造消息并入队
        rpc_msg_t *msg = malloc(sizeof(rpc_msg_t));
        if (!msg) 
        {
            free(json_str);
            break;
        }

        msg->client_fd = client_fd;
        msg->request = json_str;      
        msg->response = NULL;
        queue_push(recv_queue, msg);
    }

    close(client_fd);
}

// 接收线程
static void *recv_thread(void *arg)
{
    rpc_on_message_cb cb = (rpc_on_message_cb)arg;

    while (g_running) 
    {
        int client_fd = accept(g_listen_fd, NULL, NULL);
        if (client_fd < 0) 
        {
            if (errno == EINTR) 
                continue;
            break;
        }

        handle_client_connection(client_fd);
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
        cb(msg->request, strlen(msg->request), &(msg->response));

        queue_push(send_queue, msg); 
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

        if (msg->response && msg->client_fd >= 0) 
        {
            unsigned int len = strlen(msg->response);
            if (len > MAX_RPC_MSG_SIZE) 
            {
                LOGE("Response too long: %u", len);
            } 
            else 
            {
                unsigned int net_len = htole32(len);  // 主机序 → 小端

                // 先发 4 字节长度头
                ssize_t w1 = write(msg->client_fd, &net_len, sizeof(net_len));
                // 再发 JSON 数据
                ssize_t w2 = write(msg->client_fd, msg->response, len);

                if (w1 != sizeof(net_len) || w2 != len) 
                {
                    LOGE("Failed to send response to client_fd=%d", msg->client_fd);
                } 
                else 
                {
                    LOGD("Sent response (%u bytes) to client_fd=%d", len, msg->client_fd);
                }
            }
        }

        if (msg->client_fd >= 0) 
        {
            close(msg->client_fd);
        }
        free(msg->request);
        free(msg->response);
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
        LOGE("socket error: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

    if(bind(g_listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
    {
        LOGE("bind error: %s", strerror(errno));
        return -1;
    }
    if(listen(g_listen_fd, 8) < 0) 
    {
        LOGE("listen error: %s", strerror(errno));
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

int rpc_make_response(int status, const char *msg, const char *data_json, char **resp)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", status);
    cJSON_AddStringToObject(root, "msg", msg);

    if (data_json) 
    {
        cJSON *data = cJSON_Parse(data_json);
        if (data) 
            cJSON_AddItemToObject(root, "data", data);
        else      
            cJSON_AddObjectToObject(root, "data");
    } 
    else 
    {
        cJSON_AddObjectToObject(root, "data");
    }

    *resp = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return 0;
}

int rpc_make_error(char **resp, int status, const char *msg)
{
    return rpc_make_response(status, msg, NULL, resp);
}