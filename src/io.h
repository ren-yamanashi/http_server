#ifndef _IO_H
#define _IO_H

typedef struct {
    char key[256];
    char value[256];
} KeyValue;

int parseJson(char *json, KeyValue *keyValue, int pairsCount);
unsigned int getFileSize(const char *);

#endif