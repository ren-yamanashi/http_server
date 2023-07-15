#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"
#include "router.h"
#include "main.h"

int main(void)
{
    // NOTE: ルートを定義
    Route routes[] = {
        {HTTP_METHOD_GET, "/", CONTENT_TYPE_HTML, "/index.html", "", NULL},
        {HTTP_METHOD_GET, "/user/:id", CONTENT_TYPE_HTML, "/index.html", "", requestHandler},
        {HTTP_METHOD_GET, "/user/:id/textbook/:textbookId", CONTENT_TYPE_HTML, "/index.html", "", requestHandler},
        {HTTP_METHOD_POST, "/test", CONTENT_TYPE_HTML, "/test.html", "", requestHandler},
        {HTTP_METHOD_POST, "/plain", CONTENT_TYPE_PLAIN, "", "hello world!", requestHandler},
    };

    // NOTE: HTTPサーバーに接続してエラーを確認
    int res = connectHttpServer(routes, sizeof(routes) / sizeof(routes[0]));
    if (res < 0)
    {
        fprintf(stderr, "Failed to connect to the HTTP server. Error code: %d\n", res);
        return 1;
    }
    return 0;
}

void requestHandler(const HttpRequest *const request, const HttpResponse *const response)
{
    // NOTE: リクエストボディを出力
    for (int i = 0; i < request->parsed_kv_count; i++)
    {
        printf("Request Body: {%s: %s}\n", request->parsed_kv[i].key, request->parsed_kv[i].value);
    }
    // NOTE: リクエストパラメータを出力
    for (int i = 0; i < request->param_kv_count; i++)
    {
        printf("Request param: {%s: %s}\n", request->param_kv[i].key, request->param_kv[i].value);
    }
}