#ifndef _RESPONSE_H
#define _RESPONSE_H

#include "struct.h"

// NOTE: 受信時、送信時の動作の詳細設定: 今回は特別なフラグを設定しないので`0`とする
#define SEND_FLAG 0

char *getStatusMessage(int status);
int createResponseMessage(char *response_message, char *header, HttpResponse *response);

#endif