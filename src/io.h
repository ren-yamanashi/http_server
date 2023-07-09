#ifndef _IO_H
#define _IO_H

typedef struct
{
    char key[256];
    char value[256];
} KeyValue;

int parseJson(char *json, KeyValue *key_value, int pairs_count);
unsigned int getFileSize(const char *);

#endif