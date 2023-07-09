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

int createResponseMessage(char *response_message, HttpResponse *response, char *header, char *body);
int sendResponseMessage(int, char *, unsigned int);

#endif