#ifndef _RESPONSE_H
#define _RESPONSE_H

// NOTE: 受信時、送信時の動作の詳細設定: 今回は特別なフラグを設定しないので`0`とする
#define SEND_FLAG 0
#define HTTP_OK 200
#define HTTP_NOT_FOUND 404
#define HTTP_OK_MESSAGE "OK"
#define HTTP_NOT_FOUND_MESSAGE "Not Found"
#define HTTP_VERSION "HTTP/1.1"

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