#ifndef _ROUTER_H
#define _ROUTER_H

#include "response.h"
#include "request.h"

typedef struct
{
    const char *method;
    const char *path;
    const char *content_type;
    const char *file_path;
    const char *message;
    void (*handler)(const HttpRequest * const request, const HttpResponse * const response);
} Route;

#endif