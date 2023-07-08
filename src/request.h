#ifndef _REQUEST_H
#define _REQUEST_H

typedef struct
{
    char method[32];
    char target[1024];
    char version[32];
    char contentType[128];
    char body[1024];
} HttpRequest;

int parseRequestMessage(char *input, HttpRequest *request); 
int checkRequestMethod(const char *req_method);
int recvRequestMessage(int, char *, unsigned int);
int processingRequest(char *, char *);

#endif