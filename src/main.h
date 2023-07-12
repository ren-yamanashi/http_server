#ifndef _MAIN_H
#define _MAIN_H

#include "server.h"
#include "router.h"

int main(void);
void requestHandler(const HttpRequest *const request, const HttpResponse *const response);

#endif