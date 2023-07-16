#include <stdio.h>
#include "struct.h"
#include "lib.h"

void createRoute(Route *route, const char *method, const char *path,
                 const char *content_type, const char *file_path,
                 const char *message, void (*handler)(const HttpRequest *const request, const HttpResponse *const response))
{
    route->method = method;
    route->path = path;
    route->content_type = content_type;
    route->file_path = file_path;
    route->message = message;
    route->handler = handler;
}

int runServer(Route *routes, int routes_count)
{
    return connectHttpServer(routes, routes_count);
}
