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
#include "router.h"
#include "io.h"

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
int httpServer(int sock, Route *route)
{
    int request_size, response_size;
    char request_message[SIZE];
    char response_message[SIZE];
    char header_field[SIZE];
    char body[SIZE];
    int status;
    unsigned int file_size;
    HttpRequest request = {0};

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