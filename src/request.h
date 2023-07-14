#ifndef _REQUEST_H
#define _REQUEST_H
#include "io.h"

// NOTE: 受信時、送信時の動作の詳細設定: 今回は特別なフラグを設定しないので`0`とする
#define RECV_FLAG 0
#define DELIM "/"

typedef struct
{
    char method[32];
    char target[1024];
    char version[32];
    char content_type[128];
    char body[1024];
    KeyValue param_kv[10];
    unsigned int param_kv_count;
    KeyValue parsed_kv[10];
    unsigned int parsed_kv_count;
} HttpRequest;

int parseRequestLine(char *line, HttpRequest *request);
int parseRequestMessage(char *input, HttpRequest *request);
int recvRequestMessage(int, char *, unsigned int);
int parseRequestBody(HttpRequest *request);
int isPathMatchRequestURL(const char *request_path, const char *route_path);
int parseRequestURL(const char *route_path, HttpRequest *request);

#endif