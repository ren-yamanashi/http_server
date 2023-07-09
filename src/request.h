#ifndef _REQUEST_H
#define _REQUEST_H
#include "io.h"
typedef struct
{
    char method[32];
    char target[1024];
    char version[32];
    char contentType[128];
    char body[1024];
    KeyValue parsedBody[10];
} HttpRequest;

int parseRequestMessage(char *input, HttpRequest *request);
int recvRequestMessage(int, char *, unsigned int);
int processingRequest(char *, char *);
int parseRequestBody(HttpRequest *request);

#endif