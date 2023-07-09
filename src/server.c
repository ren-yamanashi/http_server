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
    HttpRequest request = {0};
    HttpResponse response = {0};

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

        // NOTE: routeで設定した情報と、リクエスト内容が一致していない場合、contentTypeの値が受け入れ不可であれば404を返す
        if ((strcmp(request.method, route->method) != 0 || strcmp(request.target, route->path) != 0) || (strcmp(route->contentType, "text/html") != 0 && strcmp(route->contentType, "text/plain") != 0))
        {
            response.status = 404;
        }
        else
        {
            // NOTE: contentTypeが`text/html`の場合は、ファイルを読み込む
            if (strcmp(route->contentType, "text/html") == 0)
            {
                response.status = processingRequest(body, &route->filePath[1]);
                response.body_size = getFileSize(&route->filePath[1]);
            }
            // NOTE: contentTypeが`text/plain`の場合は、そのままbodyに格納
            else
            {
                strncpy(body, route->message, sizeof(body) - 1);
                body[sizeof(body) - 1] = '\0';
                response.status = 200;
                response.body_size = strlen(body);
            }
            strncpy(response.content_type, route->contentType, sizeof(response.content_type) - 1);
            response.content_type[sizeof(response.content_type) - 1] = '\0';
        }

        response_size = createResponseMessage(response_message, &response, header_field, body);

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