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
#include "helper.h"

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
                isPathAndURLMatch(&request.target, routes[i].path) &&
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
                response.status = readFile(response.body, &routes[matched_route].file_path[1]);
                response.body_size = getFileSize(&routes[matched_route].file_path[1]);
            }
            // NOTE: content_typeが`text/plain`の場合は、そのままbodyに格納
            else
            {
                copyStringSafely(response.body, routes[matched_route].message, sizeof(response.body));
                response.status = 200;
                response.body_size = strlen(response.body);
            }

            // NOTE: content_typeを格納
            copyStringSafely(response.content_type, routes[matched_route].content_type, sizeof(response.content_type));

            // NOTE: request_paramを格納
            extractRequestParams(routes[matched_route].path, &request);

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

int connectHttpServer(Route routes[32], int routes_count)
{
    int waiting_sock_addr, connected_sock_addr, DEFAULT_PROTOCOL = 0;
    struct sockaddr_in sock_addr_info;

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
        httpServer(connected_sock_addr, routes, routes_count);

        // NOTE: ソケット通信をクローズ
        close(connected_sock_addr);

        // NOTE: 次の接続要求を受け付ける
    }
    // NOTE: 接続待ちのソケットをクローズ
    close(waiting_sock_addr);

    return 0;
}