#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"
#include "router.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080

void requestHandler(const HttpRequest *const request, const HttpResponse *const response)
{
    for (int i = 0; i < request->parsed_kv_count; i++)
    {
        printf("Request Body: {%s: %s}\n", request->parsed_kv[i].key, request->parsed_kv[i].value);
    }

    for (int i = 0; i < request->param_kv_count; i++)
    {
        printf("Request param: {%s: %s}\n", request->param_kv[i].key, request->param_kv[i].value);
    }
}

int main(void)
{
    int waiting_sock_addr, connected_sock_addr, DEFAULT_PROTOCOL = 0;
    struct sockaddr_in sock_addr_info;
    Route routes[] = {
        {"GET", "/", "text/html", "/index.html", "", NULL},
        {"GET", "/user/:id", "text/html", "/index.html", "", requestHandler},
        {"GET", "/user/:id/textbook/:textbookId", "text/html", "/index.html", "", requestHandler},
        {"POST", "/test", "text/html", "/test.html", "", requestHandler},
        {"POST", "/plain", "text/plain", "", "hello world!", requestHandler},
    };

    // NOTE: ソケットを作成
    waiting_sock_addr = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (waiting_sock_addr == -1)
    {
        printf("socket error\n");
        return -1;
    }

    // NOTE: 構造体を全て0にセット
    memset(&sock_addr_info, 0, sizeof(struct sockaddr_in));

    /** サーバーのIPアドレスとボート番号の情報を設定 */
    // NOTE: アドレスファミリーを指定
    sock_addr_info.sin_family = AF_INET;
    // NOTE: 使用するポート番号を指定
    sock_addr_info.sin_port = htons((unsigned short)SERVER_PORT);
    // NOTE: 使用するIPアドレスを指定
    sock_addr_info.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // NOTE: ソケットを特定のネットワークアドレス（IPアドレスとポート番号の組）に紐付ける
    if (bind(waiting_sock_addr, (const struct sockaddr *)&sock_addr_info, sizeof(sock_addr_info)) == -1)
    {
        printf("bind error\n");
        close(waiting_sock_addr);
        return -1;
    }

    // NOTE: ソケットを接続待ちに設定
    if (listen(waiting_sock_addr, 3) == -1)
    {
        printf("listen error\n");
        close(waiting_sock_addr);
        return -1;
    }

    while (1)
    {
        printf("Waiting connect...\n");

        // NOTE: 接続を受け付ける
        connected_sock_addr = accept(waiting_sock_addr, NULL, NULL);
        if (connected_sock_addr == -1)
        {
            printf("accept error\n");
            close(waiting_sock_addr);
            return -1;
        }
        printf("Connected!!\n");

        // NOTE: 接続済みのソケットでデータのやり取り
        httpServer(connected_sock_addr, routes, sizeof(routes) / sizeof(routes[0]));

        // NOTE: ソケット通信をクローズ
        close(connected_sock_addr);

        // NOTE: 次の接続要求を受け付ける
    }
    // NOTE: 接続待ちのソケットをクローズ
    close(waiting_sock_addr);
    return 0;
}