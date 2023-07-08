#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"
#include "response.h"
#include "request.h"
#include "io.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define SIZE (5 * 1024)

/**
 * 受信した文字列を表示
 * @param message メッセージを格納するバッファへのアドレス
 * @param size メッセージのバイト数
 */
void showMessage(char *message, unsigned int size)
{
    unsigned int i;

    for (i = 0; i < size; i++)
    {
        putchar(message[i]);
    }
    printf("\n\n");
}

/**
 * HTTPサーバーの処理を行う関数
 * @param sock: 接続済みのソケット
 * @return 0
 */
int httpServer(int sock)
{
    int request_size, response_size;
    char request_message[SIZE];
    char response_message[SIZE];
    char header_field[SIZE];
    char body[SIZE];
    int status;
    unsigned int file_size;
    HttpRequest request;

    while (1)
    {
        request_size = recvRequestMessage(sock, request_message, SIZE);
        if (request_size == -1)
        {
            printf("recvRequestMessage error\n");
            break;
        }

        if (request_size == 0)
        {
            // NOTE: 受信サイズが0の場合は相手が接続を閉じていると判断
            printf("connection ended\n");
            break;
        }

        printf("\nShow Request Message \n\n");
        showMessage(request_message, request_size);

        if (parseRequestMessage(request_message, &request) == -1)
        {
            printf("parseRequestMessage error\n");
            break;
        }

        // NOTE: requestMethodが受信可能なものか判別
        if (checkRequestMethod(request.method) != 0)
        {
            status = 404;
        }
        else
        {
            if (strcmp(request.target, "/") == 0)
            {
                // NOTE: `/`が指定されたときは`/index.html`に置き換える
                strcpy(request.target, "/index.html");
            }
            else
            {
                // NOTE: とりあえず、`~/hoge`リクエストに対して、`hoge.html`ファイルを返す
                strcat(request.target, ".html");
            }
            status = processingRequest(body, &request.target[1]);
        }

        file_size = getFileSize(&request.target[1]);
        response_size = createResponseMessage(response_message, status, header_field, body, file_size);
        if (response_size == -1)
        {
            printf("createResponseMessage error\n");
            break;
        }

        // NOTE: 送信するメッセージを表示
        printf("\nShow Response Message \n\n");
        showMessage(response_message, response_size);

        // NOTE: レスポンスメッセージを送信する
        sendResponseMessage(sock, response_message, response_size);
    }
    return 0;
}

int main(void)
{
    int w_addr, c_sock, DEFAULT_PROTOCOL = 0;
    struct sockaddr_in a_addr;

    // NOTE: ソケットを作成
    w_addr = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (w_addr == -1)
    {
        printf("socket error\n");
        return -1;
    }

    // NOTE: 構造体を全て0にセット
    memset(&a_addr, 0, sizeof(struct sockaddr_in));

    /** サーバーのIPアドレスとボート番号の情報を設定 */
    // NOTE: アドレスファミリーを指定
    a_addr.sin_family = AF_INET;
    // NOTE: 使用するポート番号を指定
    a_addr.sin_port = htons((unsigned short)SERVER_PORT);
    // NOTE: 使用するIPアドレスを指定
    a_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // NOTE: ソケットを特定のネットワークアドレス（IPアドレスとポート番号の組）に紐付ける
    if (bind(w_addr, (const struct sockaddr *)&a_addr, sizeof(a_addr)) == -1)
    {
        printf("bind error\n");
        close(w_addr);
        return -1;
    }

    // NOTE: ソケットを接続待ちに設定
    if (listen(w_addr, 3) == -1)
    {
        printf("listen error\n");
        close(w_addr);
        return -1;
    }

    while (1)
    {
        printf("Waiting connect...\n");

        // NOTE: 接続を受け付ける
        c_sock = accept(w_addr, NULL, NULL);
        if (c_sock == -1)
        {
            printf("accept error\n");
            close(w_addr);
            return -1;
        }
        printf("Connected!!\n");

        // NOTE: 接続済みのソケットでデータのやり取り
        httpServer(c_sock);

        // NOTE: ソケット通信をクローズ
        close(c_sock);

        // NOTE: 次の接続要求を受け付ける
    }
    // NOTE: 接続待ちのソケットをクローズ
    close(w_addr);
    return 0;
}