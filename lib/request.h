#ifndef _REQUEST_H
#define _REQUEST_H

#include "struct.h"

// NOTE: 受信時、送信時の動作の詳細設定: 今回は特別なフラグを設定しないので`0`とする
#define RECV_FLAG 0
#define DELIM "/"

int parseRequestLine(char *line, HttpRequest *request);
int parseRequestMessage(char *input, HttpRequest *request);
int recvRequestMessage(int, char *, unsigned int);
int parseRequestBody(HttpRequest *request);
int isPathAndURLMatch(const char *request_path, const char *route_path);
void extractRequestParams(const char *route_path, HttpRequest *request);
void parseRequestHeader(char *line, char *line_save, HttpRequest *request);
char *splitRequestHeaderAndBody(char *request_message);

#endif