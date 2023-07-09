#ifndef _RESPONSE_H
#define _RESPONSE_H

typedef struct
{
    int status;
    char message[32];
    int contentLength;
    char *body;
} HttpResponse;

int createResponseMessage(char *response_message, int status, char *header, char *body, unsigned int body_size, char *content_type);
int sendResponseMessage(int, char *, unsigned int);

#endif