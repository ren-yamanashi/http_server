#ifndef _SERVER_H
#define _SERVER_H

#include "router.h"

int httpServer(int sock, Route *routes, int routesCount);
void showMessage(char *message, unsigned int size);

#endif