#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/dev.sock"
#define MAX_MSG_SIZE 8192

// 小端转换（假设运行在小端机器上，如 x86/ARM）
static uint32_t host_to_le32(uint32_t x) {
    return x; // no-op on little-endian
}

// 读取完整响应（带长度头）
static int read_response(int fd, char *buf, size_t buf_size) {
    uint32_t net_len;
    ssize_t n = read(fd, &net_len, sizeof(net_len));
    if (n != sizeof(net_len)) {
        return -1;
    }

    uint32_t len = host_to_le32(net_len);
    if (len == 0 || len >= buf_size) {
        return -1;
    }

    n = read(fd, buf, len);
    if (n != len) {
        return -1;
    }
    buf[len] = '\0';
    return len;
}

int main()
{
    while (1) {
        int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (client_fd < 0) {
            perror("socket");
            exit(1);
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

        if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("connect");
            close(client_fd);
            sleep(1);
            continue;
        }

        char input[1024];
        printf("输入要发送的内容(JSON字符串)\n>>> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            close(client_fd);
            break;
        }

        // 去掉换行符
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) {
            close(client_fd);
            continue;
        }

        // === 构造带长度头的消息 ===
        uint32_t json_len = strlen(input);
        if (json_len == 0 || json_len > MAX_MSG_SIZE) {
            printf("[ERROR] Invalid JSON length\n");
            close(client_fd);
            continue;
        }

        uint32_t net_len = host_to_le32(json_len);

        // 先发 4 字节长度
        if (write(client_fd, &net_len, sizeof(net_len)) != sizeof(net_len)) {
            perror("write length");
            close(client_fd);
            continue;
        }

        // 再发 JSON
        if (write(client_fd, input, json_len) != json_len) {
            perror("write json");
            close(client_fd);
            continue;
        }

        // === 读取响应 ===
        char recv_buf[MAX_MSG_SIZE];
        int resp_len = read_response(client_fd, recv_buf, sizeof(recv_buf));
        if (resp_len > 0) {
            printf("[SERVER RESPONSE]: %s\n\n", recv_buf);
        } else {
            printf("[NO VALID RESPONSE]\n\n");
        }

        close(client_fd);
    }

    return 0;
}