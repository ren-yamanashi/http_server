#ifndef LIB_SERVER_H
#define LIB_SERVER_H

#define DATA_BLOCK_SIZE_FOR_READ 1
#define SEND_FLAG 0
#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define SOCK_DEFAULT_PROTOCOL 0
#define HTTP_STATUS_OK 200
#define HTTP_STATUS_NOT_FOUND 404
#define HTTP_STATUS_ERROR 500
#define HTTP_OK_MESSAGE "OK"
#define HTTP_NOT_FOUND_MESSAGE "Not Found"
#define HTTP_SERVER_ERROR "Error"
#define HTTP_VERSION "HTTP/1.1"
#define ERROR_FLAG -1
#define SUCCESS_FLAG 0
#define NUM_OF_CONNECT_KEEP 3
#define SIZE (5 * 1024)
#define RECV_FLAG 0
#define DELIM "/"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

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

void createRoute(Route *route, const char *method, const char *path,
                 const char *content_type, const char *file_path,
                 const char *message, void (*handler)(const HttpRequest *const request, const HttpResponse *const response));
int runServer(Route *routes, int routes_count);
int httpServer(int sock, Route *routes, int routes_count);
void showMessage(char *message, unsigned int size);
int connectHttpServer(Route routes[32], int routes_count);
void processResponse(int sock, HttpResponse *response);
int processRequest(int sock, HttpRequest *request, HttpResponse *response);
void setResponseInfo(Route *route, HttpResponse *response);
int parseRequestLine(char *line, HttpRequest *request);
int parseRequestMessage(char *input, HttpRequest *request);
int recvRequestMessage(int, char *, unsigned int);
int parseRequestBody(HttpRequest *request);
int isPathAndURLMatch(const char *request_path, const char *route_path);
void extractRequestParams(const char *route_path, HttpRequest *request);
void parseRequestHeader(char *line, char *line_save, HttpRequest *request);
char *splitRequestHeaderAndBody(char *request_message);
char *getStatusMessage(int status);
int createResponseMessage(char *response_message, char *header, HttpResponse *response);
int parseJson(char *json, KeyValue *key_value, int pairs_count);
int getFileSize(const char *);
int readFile(char *body, const char *file_path);
void copyStringSafely(char *destination, char *source, size_t destination_size);
int isError(int target);
int isMatchStr(const char *str1, const char *str2);

#endif