#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "request.h"
#include "io.h"
#include "helper.h"

/**
 * リクエストメッセージを受信する
 * @param sock 接続済みのソケット
 * @param request_message リクエストメッセージを格納するバッファへのアドレス
 * @param buf_size リクエストメッセージを格納するバッファのサイズ
 * @return 実際に接続先から受信したデータのバイト数
 */
int recvRequestMessage(int sock, char *request_message, unsigned int buf_size)
{
    // NOTE: リクエストを受信
    int recv_size = recv(sock, request_message, buf_size, RECV_FLAG);
    if (recv_size < 0)
    {
        perror("Error: Failed to receive request message");
        return -1;
    }
    // NOTE: バッファの現在の終端をNULL文字で終了
    request_message[recv_size] = '\0';
    return recv_size;
}

/**
 * リクエストラインを解析
 * @param line リクエストメッセージの1行目
 * @param request HttpRequest構造体のアドレス
 * @return 成功した場合は0 それ以外は1
 */
int parseRequestLine(char *line, HttpRequest *request)
{
    char *header_save;
    // NOTE: 1行目の情報を取得
    char *req_method = strtok_r(line, " ", &header_save);
    char *req_target = strtok_r(NULL, " ", &header_save);
    char *version = strtok_r(NULL, " ", &header_save);
    if (req_method == NULL || req_target == NULL || version == NULL)
    {
        printf("Error: Could not parse the request line\n");
        return -1;
    }
    // NOTE: 1行目の情報を構造体に格納
    strncpy(request->method, req_method, sizeof(request->method) - 1);
    strncpy(request->target, req_target, sizeof(request->target) - 1);
    strncpy(request->version, version, sizeof(request->version) - 1);
    return 0;
}

/**
 * リクエストヘッダを解析
 * 特定の文字列を見つけて、それに該当する処理を行う
 * @param line リクエストメッセージの行
 * @param line_save
 * @param request HttpRequest構造体のアドレス
 * @return
 */
void parseRequestHeader(char *line, char *line_save, HttpRequest *request)
{
    char *header_save, *header, *header_value;
    while (line)
    {
        header = strtok_r(line, ":", &header_save);
        header_value = strtok_r(NULL, "", &header_save);
        if (header && header_value && strcmp(header, "Content-Type") == 0)
        {
            // NOTE: コンテンツタイプを取得
            // NOTE: `:`の後のスペースをスキップ
            header_value++;
            copyStringSafely(request->content_type, header_value, sizeof(request->content_type));
        }

        // NOTE: 行の取得を繰り返す
        line = strtok_r(NULL, "\r\n", &line_save);
    }
}

/**
 * リクエストメッセージを解析する
 * @param request_message 解析するリクエストメッセージが格納されたバッファへのアドレス
 * @param request HttpRequestの構造体
 * @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
 */
char *splitRequestHeaderAndBody(char *request_message)
{
    char *headers_end = strstr(request_message, "\r\n\r\n");
    if (!headers_end)
    {
        printf("Error: Could not find end of headers\n");
        return NULL;
    }
    // NOTE: ヘッダー部分をnullで終了し、ボディを「\r\n\r\n」の後に開始するように設定
    *headers_end = "\0";
    return headers_end + 4;
}

/**
 * リクエストメッセージを解析する
 * @param request_message 解析するリクエストメッセージが格納されたバッファへのアドレス
 * @param request HttpRequestの構造体
 * @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
 */
int parseRequestMessage(char *request_message, HttpRequest *request)
{
    char *line, *line_save;

    // NOTE: ヘッダーとボディを分離
    char *body_start = splitRequestHeaderAndBody(request_message);
    if (body_start == NULL)
    {
        return -1;
    }

    // NOTE: リクエストメッセージの1行目を取得
    line = strtok_r(request_message, "\r\n", &line_save);
    if (line == NULL)
    {
        printf("Error: Could not get request\n");
        return -1;
    }

    parseRequestLine(line, request);
    line = strtok_r(NULL, "\r\n", &line_save);
    parseRequestHeader(line, line_save, request);
    snprintf(request->body, sizeof(request->body), "%s", body_start);
    return parseRequestBody(request);
}

/**
 * リクエストボディを解析
 * jsonの場合は、objectに変換
 * plainTextの場合は、何もしない
 * @param request HttpRequest構造体のアドレス
 * @return 成功した場合は0 それ以外は-1
 */
int parseRequestBody(HttpRequest *request)
{
    if (strlen(request->content_type) == 0)
    {
        return 0;
    }
    // NOTE: リクエストヘッダのContent-Typeが `text/plain`,`application/json` 以外はエラー
    if (strcmp(request->content_type, "application/json") != 0 && strcmp(request->content_type, "text/plain") != 0)
    {
        printf("Error: Failed to parse request header\n");
        return -1;
    }
    // NOTE: リクエストヘッダのContent-Typeが `text/plain` の場合
    if (strcmp(request->content_type, "text/plain") == 0)
    {
        return 0;
    }
    // NOTE: リクエストヘッダのContent-Typeが `application/json` の場合
    request->parsed_kv_count = parseJson(request->body, request->parsed_kv, sizeof(request->parsed_kv) / sizeof(KeyValue));
    if (request->parsed_kv_count < 0)
    {
        printf("Error: Failed to parse JSON\n");
        return -1;
    }
    return 0;
}

/**
 * リクエストURLにパラメータが含まれる場合に、routeで設定したpathを一致するかを確認する
 * @param request_url リクエストURLを示すアドレス
 * @param route_path ルートのパスを示すアドレス
 * @return 一致した場合は1 そうでない場合は0
 */
int isPathAndURLMatch(const char *request_url, const char *route_path)
{
    char route_copy[1024];
    char request_copy[1024];
    char *route_saveptr;
    char *request_saveptr;
    strcpy(route_copy, route_path);
    strcpy(request_copy, request_url);
    char *route_token = strtok_r(route_copy, DELIM, &route_saveptr);
    char *request_token = strtok_r(request_copy, DELIM, &request_saveptr);

    while (route_token != NULL && request_token != NULL)
    {
        if (route_token[0] != ':' && strcmp(route_token, request_token) != 0)
        {
            return 0;
        }
        route_token = strtok_r(NULL, DELIM, &route_saveptr);
        request_token = strtok_r(NULL, DELIM, &request_saveptr);
    }

    if (route_token != NULL || request_token != NULL)
    {
        return 0;
    }

    return 1;
}

/**
 * リクエストURLを解析して、urlパラメータを取得する
 * @param request HttpRequest構造体を示すアドレス
 * @param route_path routeで指定したpathを示すアドレス
 * @return
 */
int extractRequestParams(const char *route_path, HttpRequest *request)
{
    char route_copy[1024];
    char request_copy[1024];
    char *route_saveptr;
    char *request_saveptr;
    strcpy(route_copy, route_path);
    strcpy(request_copy, request->target);
    char *route_token = strtok_r(route_copy, DELIM, &route_saveptr);
    char *request_token = strtok_r(request_copy, DELIM, &request_saveptr);
    int param_kv_count = 0;

    while (route_token != NULL && request_token != NULL)
    {
        if (route_token[0] == ':')
        {
            // NOTE: `:`は不要なので、2文字目以降を格納
            copyStringSafely(request->param_kv[param_kv_count].key, &route_token[1], sizeof(request->param_kv[param_kv_count].key));
            copyStringSafely(request->param_kv[param_kv_count].value, request_token, sizeof(request->param_kv[param_kv_count].value));
            param_kv_count++;
        }
        route_token = strtok_r(NULL, DELIM, &route_saveptr);
        request_token = strtok_r(NULL, DELIM, &request_saveptr);
    }
    request->param_kv_count = param_kv_count;
    return 0;
}