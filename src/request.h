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
    KeyValue parsed_kv[10];
    unsigned int kv_count;
} HttpRequest;

int parseRequestMessage(char *input, HttpRequest *request);
int recvRequestMessage(int, char *, unsigned int);
int parseRequestBody(HttpRequest *request);

#endif