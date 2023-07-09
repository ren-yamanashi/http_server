#ifndef _ROUTER_H
#define _ROUTER_H

#include "response.h"
#include "request.h"

typedef struct
{
    const char *method;
    const char *path;
    const char *contentType;
    const char *filePath;
    const char *message;
} Route;

#endif