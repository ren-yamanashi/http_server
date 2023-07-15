#ifndef _SERVER_H
#define _SERVER_H

#include "router.h"

#define SIZE (5 * 1024)
#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080

int httpServer(int sock, Route *routes, int routes_count);
void showMessage(char *message, unsigned int size);
int connectHttpServer(Route routes[32], int routes_count);
void processResponse(int sock, HttpResponse *response);
int processRequest(int sock, HttpRequest *request, HttpResponse *response);
void setResponseInfo(Route *route, HttpResponse *response);

#endif