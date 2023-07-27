#ifndef _STRUCT_H
#define _STRUCT_H

typedef struct
{
    char key[256];
    char value[256];
} KeyValue;

typedef struct
{
    int status;
    char content_type[32];
    int content_length;
    unsigned int body_size;
    char body[1024];
} HttpResponse;

typedef struct
{
    char method[32];
    char target[1024];
    char version[32];
    char content_type[128];
    char body[1024];
    KeyValue param_kv[10];
    unsigned int param_kv_count;
    KeyValue parsed_kv[10];
    unsigned int parsed_kv_count;
} HttpRequest;

typedef struct
{
    const char *method;
    const char *path;
    const char *content_type;
    const char *file_path;
    const char *message;
    void (*handler)(const HttpRequest *const request, const HttpResponse *const response);
} Route;

#endif