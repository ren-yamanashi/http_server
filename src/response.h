#ifndef _RESPONSE_H
#define _RESPONSE_H

// NOTE: 受信時、送信時の動作の詳細設定: 今回は特別なフラグを設定しないので`0`とする
#define SEND_FLAG 0

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

#endif