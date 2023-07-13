#ifndef _RESPONSE_H
#define _RESPONSE_H

typedef struct
{
    int status;
    char content_type[32];
    int content_length;
    unsigned int body_size;
    char body[1024];
} HttpResponse;

char *getStatusMessage(int status);
int createResponseMessage(char *response_message, char *header, HttpResponse *response);
int sendResponseMessage(int, char *, unsigned int);

#endif