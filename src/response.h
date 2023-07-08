#ifndef _RESPONSE_H
#define _RESPONSE_H

typedef struct
{
    int status;
    char message[32];
    int contentLength;
    char *body;
} HttpResponse;

int createResponseMessage(char *, int, char *, char *, unsigned int);
int sendResponseMessage(int, char *, unsigned int);

#endif