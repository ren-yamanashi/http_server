#ifndef _IO_H
#define _IO_H

#define SIZE (5 * 1024)
#define DATA_BLOCK_SIZE_FOR_READ 1

typedef struct
{
    char key[256];
    char value[256];
} KeyValue;

int parseJson(char *json, KeyValue *key_value, int pairs_count);
int getFileSize(const char *);
int readFile(char *body, const char *file_path);
void copyStringSafely(char *destination, char *source, size_t destination_size);

#endif