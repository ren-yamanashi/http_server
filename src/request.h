#ifndef _REQUEST_H
#define _REQUEST_H
#include "io.h"
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

int parseRequestMessage(char *input, HttpRequest *request);
int recvRequestMessage(int, char *, unsigned int);
int parseRequestBody(HttpRequest *request);
int isPathMatch(const char *request_path, const char *route_path);
int parseRequestURL(const char *route_path, HttpRequest *request);
#endif