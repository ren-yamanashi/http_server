#ifndef _SERVER_H
#define _SERVER_H

#include "struct.h"

int httpServer(int sock, Route *routes, int routes_count);
void showMessage(char *message, unsigned int size);
int connectHttpServer(Route routes[32], int routes_count);
void processResponse(int sock, HttpResponse *response);
int processRequest(int sock, HttpRequest *request, HttpResponse *response);
void setResponseInfo(Route *route, HttpResponse *response);

#endif