#ifndef _REQUEST_H
#define _REQUEST_H

int checkRequestMethod(const char *req_method);
int recvRequestMessage(int, char *, unsigned int);
int parseRequestMessage(char *, char *, char *);
int processingRequest(char *, char *);

#endif