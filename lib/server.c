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
#include "helper.h"
#include "constance.h"
#include "struct.h"

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
 * リクエストに対する処理
 * @param sock 対象のソケット
 * @param request HttpRequest構造体のアドレス
 * @param response HttpResponse構造体のアドレス
 * @return 成功した場合は0 それ以外は-1
 */
int processRequest(int sock, HttpRequest *request, HttpResponse *response)
{
    char request_message[SIZE];
    int request_size = recvRequestMessage(sock, request_message, SIZE);
    if (isError(request_size))
    {
        printf("Error: Failed to receive request message\n");
        return ERROR_FLAG;
    }
    if (request_size == 0)
    {
        // NOTE: 受信サイズが0の場合は相手が接続を閉じていると判断
        printf("Info: Connection ended\n");
        return ERROR_FLAG;
    }

    printf("\n======Request message======\n\n");
    showMessage(request_message, request_size);

    if (isError(parseRequestMessage(request_message, request)))
    {
        printf("Error: Failed to parse request message\n");
        return ERROR_FLAG;
    }

    return SUCCESS_FLAG;
}

/**
 * レスポンスに対する処理
 * @param sock 対象のソケット
 * @param response HttpResponse構造体のアドレス
 */
void processResponse(int sock, HttpResponse *response)
{
    char response_message[SIZE];
    char header_field[SIZE];
    int response_size = createResponseMessage(response_message, header_field, response);
    if (isError(response_size))
    {
        printf("Error: Failed to create response message\n");
        return;
    }
    printf("\n======Response message======\n\n");
    showMessage(response_message, response_size);
    send(sock, response_message, response_size, SEND_FLAG);
}

/**
 * レスポンスの情報を構造体に格納
 * @param route Route構造体のアドレス
 * @param response HttpResponse構造体のアドレス
 */
void setResponseInfo(Route *route, HttpResponse *response)
{
    // NOTE: content_typeが`text/html`の場合は、ファイルを読み込む
    if (isMatchStr(route->content_type, "text/html"))
    {
        response->status = readFile(response->body, &route->file_path[1]);
        response->body_size = getFileSize(&route->file_path[1]);
    }
    // NOTE: content_typeが`text/plain`の場合は、そのままbodyに格納
    else
    {
        copyStringSafely(response->body, route->message, sizeof(response->body));
        response->status = HTTP_STATUS_OK;
        response->body_size = strlen(response->body);
    }
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
    int matched_route = -1;
    HttpRequest request = {0};
    HttpResponse response = {0};

    while (1)
    {
        if (isError(processRequest(sock, &request, &response)))
        {
            return 0;
        }

        for (int i = 0; i < routes_count; i++)
        {
            if (isMatchStr(request.method, routes[i].method) &&
                isPathAndURLMatch(request.target, routes[i].path) &&
                (isMatchStr(routes[i].content_type, "text/html") || isMatchStr(routes[i].content_type, "text/plain")))
            {
                matched_route = i;
                break;
            }
        }

        // NOTE: routeで設定した情報と、リクエスト内容が一致していない場合、content_typeの値が受け入れ不可であれば404を返す
        if (isError(matched_route))
        {
            response.status = HTTP_STATUS_NOT_FOUND;
        }
        else
        {
            // NOTE: レスポンスの情報を格納
            setResponseInfo(&routes[matched_route], &response);
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
        processResponse(sock, &response);
    }
    return 0;
}

int connectHttpServer(Route routes[32], int routes_count)
{
    int waiting_sock_addr, connected_sock_addr;
    struct sockaddr_in sock_addr_info;

    // NOTE: ソケットを作成
    waiting_sock_addr = socket(AF_INET, SOCK_STREAM, SOCK_DEFAULT_PROTOCOL);
    if (isError(waiting_sock_addr))
    {
        printf("Error: Failed create socket\n");
        return ERROR_FLAG;
    }

    // NOTE: 構造体を全て0にセット
    memset(&sock_addr_info, 0, sizeof(struct sockaddr_in));

    // NOTE:サーバーのアドレスファミリー・IPアドレス・ボート番号の情報を設定
    // NOTE: アドレスファミリーを指定
    sock_addr_info.sin_family = AF_INET;
    // NOTE: 使用するポート番号を指定
    sock_addr_info.sin_port = htons((unsigned short)SERVER_PORT);
    // NOTE: 使用するIPアドレスを指定
    sock_addr_info.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // NOTE: ソケットを特定のネットワークアドレス（IPアドレスとポート番号の組）に紐付ける
    if (isError(bind(waiting_sock_addr, (const struct sockaddr *)&sock_addr_info, sizeof(sock_addr_info))))
    {
        printf("Error: Failed to bind socket with network address\n");
        close(waiting_sock_addr);
        return ERROR_FLAG;
    }

    // NOTE: ソケットを接続待ちに設定
    if (isError(listen(waiting_sock_addr, NUM_OF_CONNECT_KEEP)))
    {
        printf("Error: Could not set socket to listen for connection\n");
        close(waiting_sock_addr);
        return ERROR_FLAG;
    }

    while (1)
    {
        // NOTE: 接続を受け付ける
        printf("Info: Waiting connect...\n");
        connected_sock_addr = accept(waiting_sock_addr, NULL, NULL);
        if (isError(connected_sock_addr))
        {
            printf("Error: Failed to accept connection\n");
            close(waiting_sock_addr);
            return ERROR_FLAG;
        }
        printf("Info: Success connected!!\n");
        // NOTE: 接続済みのソケットでデータのやり取り
        httpServer(connected_sock_addr, routes, routes_count);
        // NOTE: ソケット通信をクローズ
        close(connected_sock_addr);
        // NOTE: 次の接続要求を受け付ける
    }
    // NOTE: 接続待ちのソケットをクローズ
    close(waiting_sock_addr);

    return SUCCESS_FLAG;
}