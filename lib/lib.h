#ifndef LIB_H
#define LIB_H

#include "server.h"

void createRoute(Route *route, const char *method, const char *path,
                 const char *content_type, const char *file_path,
                 const char *message, void (*handler)(const HttpRequest *const request, const HttpResponse *const response));
int runServer(Route *routes, int routes_count);

#endif