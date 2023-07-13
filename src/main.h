#ifndef _MAIN_H
#define _MAIN_H

#include "server.h"
#include "router.h"

#define HTTP_METHOD_GET "GET"
#define HTTP_METHOD_POST "POST"
#define CONTENT_TYPE_HTML "text/html"
#define CONTENT_TYPE_PLAIN "text/plain"

int main(void);
void requestHandler(const HttpRequest *const request, const HttpResponse *const response);

#endif