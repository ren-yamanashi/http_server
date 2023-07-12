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
    Route routes[] = {
        {"GET", "/", "text/html", "/index.html", "", NULL},
        {"GET", "/user/:id", "text/html", "/index.html", "", requestHandler},
        {"GET", "/user/:id/textbook/:textbookId", "text/html", "/index.html", "", requestHandler},
        {"POST", "/test", "text/html", "/test.html", "", requestHandler},
        {"POST", "/plain", "text/plain", "", "hello world!", requestHandler},
    };

    int res = connectHttpServer(routes, sizeof(routes) / sizeof(routes[0]));
    return res;
}

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