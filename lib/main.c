#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"
#include "main.h"
#include "struct.h"
#include "lib.h"

int main(void)
{
    // NOTE: ルートを定義
    Route routes[6];
    createRoute(&routes[0], HTTP_METHOD_GET, "/", CONTENT_TYPE_HTML, "/index.html", "", NULL);
    createRoute(&routes[1], HTTP_METHOD_GET, "/user/:id", CONTENT_TYPE_HTML, "/index.html", "", requestHandler);
    createRoute(&routes[2], HTTP_METHOD_GET, "/user/:id/textbook/:textbookId", CONTENT_TYPE_HTML, "/index.html", "", requestHandler);
    createRoute(&routes[3], HTTP_METHOD_POST, "/test", CONTENT_TYPE_HTML, "/index.html", "", requestHandler);
    createRoute(&routes[4], HTTP_METHOD_POST, "/plain", CONTENT_TYPE_PLAIN, "", "hello world!", requestHandler);
    createRoute(&routes[5], HTTP_METHOD_DELETE, "/data/:id/delete", CONTENT_TYPE_PLAIN, "", "delete data", requestHandler);

    // NOTE: HTTPサーバーに接続してエラーを確認
    int res = runServer(routes, sizeof(routes) / sizeof(routes[0]));
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