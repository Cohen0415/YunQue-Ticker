#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/dev.sock"

int main()
{
    int client_fd;
    struct sockaddr_un addr;

    while (1) {
        client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (client_fd < 0) {
            perror("socket");
            exit(1);
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, SOCKET_PATH);

        if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("connect");
            close(client_fd);
            sleep(1);
            continue;
        }

        char input[1024];
        char recv_buf[1024];

        printf("输入要发送的内容(JSON字符串)\n>>> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) break;

        input[strcspn(input, "\n")] = 0;   // 去掉换行
        if (strlen(input) == 0) continue;

        write(client_fd, input, strlen(input));
        shutdown(client_fd, SHUT_WR);

        int n = read(client_fd, recv_buf, sizeof(recv_buf)-1);
        if (n > 0) {
            recv_buf[n] = '\0';
            printf("[SERVER RESPONSE]: %s\n\n", recv_buf);
        } else {
            printf("[NO RESPONSE]\n");
        }

        close(client_fd);
    }

    return 0;
}
