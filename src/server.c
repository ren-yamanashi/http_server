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
    printf("\r\n");
}

/**
 * HTTPサーバーの処理を行う関数
 * @param sock: 接続済みのソケット
 * @param routes: ルーティング情報を示す構造体(Route)の配列
 * @param routes_count: ルーティング情報の数
 * @return 0
 */
int httpServer(int sock, Route *routes, int routes_count)
{
    int request_size, response_size;
    int matched_route = -1;
    char request_message[SIZE];
    char response_message[SIZE];
    char header_field[SIZE];
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

        for (int i = 0; i < routes_count; i++)
        {
            if ((strcmp(request.method, routes[i].method) == 0) &&
                isPathMatch(&request.target, routes[i].path) &&
                (strcmp(routes[i].content_type, "text/html") == 0 || strcmp(routes[i].content_type, "text/plain") == 0))
            {
                matched_route = i;
                break;
            }
        }

        // NOTE: routeで設定した情報と、リクエスト内容が一致していない場合、content_typeの値が受け入れ不可であれば404を返す
        if (matched_route == -1)
        {
            response.status = 404;
        }
        else
        {
            // NOTE: content_typeが`text/html`の場合は、ファイルを読み込む
            if (strcmp(routes[matched_route].content_type, "text/html") == 0)
            {
                response.status = readFile(&response.body, &routes[matched_route].file_path[1]);
                response.body_size = getFileSize(&routes[matched_route].file_path[1]);
            }
            // NOTE: content_typeが`text/plain`の場合は、そのままbodyに格納
            else
            {
                strncpy(response.body, routes[matched_route].message, sizeof(response.body) - 1);
                response.body[sizeof(response.body) - 1] = '\0';
                response.status = 200;
                response.body_size = strlen(response.body);
            }

            // NOTE: content_typeを格納
            strncpy(response.content_type, routes[matched_route].content_type, sizeof(response.content_type) - 1);
            response.content_type[sizeof(response.content_type) - 1] = '\0';

            // NOTE: request_paramを格納
            parseRequestURL(routes[matched_route].path, &request);

            // NOTE: routeで登録したハンドラーを実行
            if (routes[matched_route].handler != NULL)
            {
                routes[matched_route].handler(&request, &response);
            }
        }

        response_size = createResponseMessage(response_message, header_field, &response);

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