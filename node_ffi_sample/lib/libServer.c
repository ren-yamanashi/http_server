#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "libServer.h"

// lib
void createRoute(Route *route, const char *method, const char *path,
                 const char *content_type, const char *file_path,
                 const char *message, void (*handler)(const HttpRequest *const request, const HttpResponse *const response))
{
    route->method = method;
    route->path = path;
    route->content_type = content_type;
    route->file_path = file_path;
    route->message = message;
    route->handler = handler;
}

int runServer(Route *routes, int routes_count)
{
    return connectHttpServer(routes, routes_count);
}

// server
/**
 * 受信した文字列を表示
 * @param message メッセージを格納するバッファへのアドレス
 * @param size メッセージのバイト数
 */
void showMessage(char *message, unsigned int size)
{
    unsigned int i;
    for (i = 0; i < size; i++)
    {
        putchar(message[i]);
    }
    printf("\r\n");
}

/**
 * リクエストに対する処理
 * @param sock 対象のソケット
 * @param request HttpRequest構造体のアドレス
 * @param response HttpResponse構造体のアドレス
 * @return 成功した場合は0 それ以外は-1
 */
int processRequest(int sock, HttpRequest *request, HttpResponse *response)
{
    char request_message[SIZE];
    int request_size = recvRequestMessage(sock, request_message, SIZE);
    if (isError(request_size))
    {
        printf("Error: Failed to receive request message\n");
        return ERROR_FLAG;
    }
    if (request_size == 0)
    {
        // NOTE: 受信サイズが0の場合は相手が接続を閉じていると判断
        printf("Info: Connection ended\n");
        return ERROR_FLAG;
    }

    printf("\n======Request message======\n\n");
    showMessage(request_message, request_size);

    if (isError(parseRequestMessage(request_message, request)))
    {
        printf("Error: Failed to parse request message\n");
        return ERROR_FLAG;
    }

    return SUCCESS_FLAG;
}

/**
 * レスポンスに対する処理
 * @param sock 対象のソケット
 * @param response HttpResponse構造体のアドレス
 */
void processResponse(int sock, HttpResponse *response)
{
    char response_message[SIZE];
    char header_field[SIZE];
    int response_size = createResponseMessage(response_message, header_field, response);
    if (isError(response_size))
    {
        printf("Error: Failed to create response message\n");
        return;
    }
    printf("\n======Response message======\n\n");
    showMessage(response_message, response_size);
    send(sock, response_message, response_size, SEND_FLAG);
}

/**
 * レスポンスの情報を構造体に格納
 * @param route Route構造体のアドレス
 * @param response HttpResponse構造体のアドレス
 */
void setResponseInfo(Route *route, HttpResponse *response)
{
    // NOTE: content_typeが`text/html`の場合は、ファイルを読み込む
    if (isMatchStr(route->content_type, "text/html"))
    {
        response->status = readFile(response->body, &route->file_path[1]);
        response->body_size = getFileSize(&route->file_path[1]);
    }
    // NOTE: content_typeが`text/plain`の場合は、そのままbodyに格納
    else
    {
        copyStringSafely(response->body, route->message, sizeof(response->body));
        response->status = HTTP_STATUS_OK;
        response->body_size = strlen(response->body);
    }
}

/**
 * HTTPサーバーの処理を行う関数
 * @param sock: 接続済みのソケット
 * @param routes: ルーティング情報を示す構造体(Route)の配列
 * @param routes_count: ルーティング情報の数
 * @return 0
 */
int httpServer(int sock, Route *routes, int routes_count)
{
    int matched_route = -1;
    HttpRequest request = {0};
    HttpResponse response = {0};

    while (1)
    {
        if (isError(processRequest(sock, &request, &response)))
        {
            return 0;
        }

        for (int i = 0; i < routes_count; i++)
        {
            if (isMatchStr(request.method, routes[i].method) &&
                isPathAndURLMatch(&request.target, routes[i].path) &&
                (isMatchStr(routes[i].content_type, "text/html") || isMatchStr(routes[i].content_type, "text/plain")))
            {
                matched_route = i;
                break;
            }
        }

        // NOTE: routeで設定した情報と、リクエスト内容が一致していない場合、content_typeの値が受け入れ不可であれば404を返す
        if (isError(matched_route))
        {
            response.status = HTTP_STATUS_NOT_FOUND;
        }
        else
        {
            // NOTE: レスポンスの情報を格納
            setResponseInfo(&routes[matched_route], &response);
            // NOTE: content_typeを格納
            copyStringSafely(response.content_type, routes[matched_route].content_type, sizeof(response.content_type));
            // NOTE: request_paramを格納
            extractRequestParams(routes[matched_route].path, &request);
            // NOTE: routeで登録したハンドラーを実行
            if (routes[matched_route].handler != NULL)
            {
                routes[matched_route].handler(&request, &response);
            }
        }
        processResponse(sock, &response);
    }
    return 0;
}

int connectHttpServer(Route routes[32], int routes_count)
{
    int waiting_sock_addr, connected_sock_addr;
    struct sockaddr_in sock_addr_info;

    // NOTE: ソケットを作成
    waiting_sock_addr = socket(AF_INET, SOCK_STREAM, SOCK_DEFAULT_PROTOCOL);
    if (isError(waiting_sock_addr))
    {
        printf("Error: Failed create socket\n");
        return ERROR_FLAG;
    }

    // NOTE: 構造体を全て0にセット
    memset(&sock_addr_info, 0, sizeof(struct sockaddr_in));

    // NOTE:サーバーのアドレスファミリー・IPアドレス・ボート番号の情報を設定
    // NOTE: アドレスファミリーを指定
    sock_addr_info.sin_family = AF_INET;
    // NOTE: 使用するポート番号を指定
    sock_addr_info.sin_port = htons((unsigned short)SERVER_PORT);
    // NOTE: 使用するIPアドレスを指定
    sock_addr_info.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // NOTE: ソケットを特定のネットワークアドレス（IPアドレスとポート番号の組）に紐付ける
    if (isError(bind(waiting_sock_addr, (const struct sockaddr *)&sock_addr_info, sizeof(sock_addr_info))))
    {
        printf("Error: Failed to bind socket with network address\n");
        close(waiting_sock_addr);
        return ERROR_FLAG;
    }

    // NOTE: ソケットを接続待ちに設定
    if (isError(listen(waiting_sock_addr, NUM_OF_CONNECT_KEEP)))
    {
        printf("Error: Could not set socket to listen for connection\n");
        close(waiting_sock_addr);
        return ERROR_FLAG;
    }

    while (1)
    {
        // NOTE: 接続を受け付ける
        printf("Info: Waiting connect...\n");
        connected_sock_addr = accept(waiting_sock_addr, NULL, NULL);
        if (isError(connected_sock_addr))
        {
            printf("Error: Failed to accept connection\n");
            close(waiting_sock_addr);
            return ERROR_FLAG;
        }
        printf("Info: Success connected!!\n");
        // NOTE: 接続済みのソケットでデータのやり取り
        httpServer(connected_sock_addr, routes, routes_count);
        // NOTE: ソケット通信をクローズ
        close(connected_sock_addr);
        // NOTE: 次の接続要求を受け付ける
    }
    // NOTE: 接続待ちのソケットをクローズ
    close(waiting_sock_addr);

    return SUCCESS_FLAG;
}

// request
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
        return ERROR_FLAG;
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
        return ERROR_FLAG;
    }
    // NOTE: 1行目の情報を構造体に格納
    copyStringSafely(request->method, req_method, sizeof(request->method));
    copyStringSafely(request->target, req_target, sizeof(request->target));
    copyStringSafely(request->version, version, sizeof(request->version));
    return SUCCESS_FLAG;
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
    char request_message_copy[SIZE];
    snprintf(request_message_copy, sizeof(request_message_copy), "%s", request_message);

    // NOTE: ヘッダーとボディを分離
    char *body_start = splitRequestHeaderAndBody(request_message);
    if (body_start == NULL)
    {
        printf("Error: Could not get request\n");
        return ERROR_FLAG;
    }
    // NOTE: リクエストメッセージの1行目を取得
    line = strtok_r(request_message_copy, "\r\n", &line_save);
    if (line == NULL)
    {
        printf("Error: Could not get request line\n");
        return ERROR_FLAG;
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
        return SUCCESS_FLAG;
    }
    // NOTE: リクエストヘッダのContent-Typeが `text/plain`,`application/json` 以外はエラー
    if (!isMatchStr(request->content_type, "application/json") && !isMatchStr(request->content_type, "text/plain"))
    {
        printf("Error: Failed to parse request header\n");
        return ERROR_FLAG;
    }
    // NOTE: リクエストヘッダのContent-Typeが `text/plain` の場合
    if (isMatchStr(request->content_type, "text/plain"))
    {
        return SUCCESS_FLAG;
    }
    // NOTE: リクエストヘッダのContent-Typeが `application/json` の場合
    request->parsed_kv_count = parseJson(request->body, request->parsed_kv, sizeof(request->parsed_kv) / sizeof(KeyValue));
    if (request->parsed_kv_count < 0)
    {
        printf("Error: Failed to parse JSON\n");
        return ERROR_FLAG;
    }
    return SUCCESS_FLAG;
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
        if (route_token[0] != ':' && !isMatchStr(route_token, request_token))
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
 */
void extractRequestParams(const char *route_path, HttpRequest *request)
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
}

// response
/**
 * ステータスに応じたメッセージを取得
 * @param stats - HTTPステータス
 * @return メッセージ
 */
char *getStatusMessage(int status)
{
    switch (status)
    {
    case HTTP_STATUS_OK:
        return HTTP_OK_MESSAGE;
    case HTTP_STATUS_NOT_FOUND:
        return HTTP_NOT_FOUND_MESSAGE;
    case HTTP_STATUS_ERROR:
        return HTTP_SERVER_ERROR;
    default:
        printf("Error: Not support status(%d)\\n", status);
        return NULL;
    }
}

/**
 * レスポンスメッセージを作成する
 * @param response_message - レスポンスメッセージを格納するバッファへのアドレス
 * @param status - ステータスコード
 * @param header - ヘッダーフィールドを格納したバッファへのアドレス
 * @param body - ボディを格納したバッファへのアドレス
 * @param body_size - ボディのサイズ
 * @return レスポンスメッセージのデータサイズ(バイト長)
 */
int createResponseMessage(char *response_message, char *header, HttpResponse *response)
{
    char content_length[50];
    response_message[0] = '\0';
    header[0] = '\0';

    sprintf(content_length, "Content-Length: %u\r\nContent-Type: %s\r\n", response->body_size, response->content_type);
    strcat(header, content_length);

    char *status_message = getStatusMessage(response->status);
    if (status_message == NULL)
    {
        return ERROR_FLAG;
    }

    // NOTE: レスポンス行とヘッダーフィールドの文字列を作成
    sprintf(response_message, "%s %d %s\r\n%s\r\n", HTTP_VERSION, response->status, status_message, header);

    unsigned int no_body_len = strlen(response_message);
    unsigned int body_len = response->body_size;

    // NOTE: ヘッダーフィールドの後ろにボディをコピー
    memcpy(&response_message[no_body_len], response->body, body_len);

    return no_body_len + body_len;
}

/**
 * ファイルサイズの取得
 * @param path - ファイルパスを指す文字列
 * @return pathを元に読み込んだファイルのサイズ
 */
int getFileSize(const char *path)
{
    int size = 0, read_size = 0;
    char read_buf[SIZE];
    FILE *file;

    // NOTE: ファイルを開く
    file = fopen(path, "rb");
    if (file == NULL)
    {
        printf("Error: opening file: %s, error: %s\n", path, strerror(errno));
        return ERROR_FLAG;
    }

    // NOTE: ファイルから指定した数・サイズのデータを読み込む
    do
    {
        read_size = fread(read_buf, 1, SIZE, file);
        size += read_size;
    } while (read_size != 0);

    // NOTE: ファイルを閉じる
    fclose(file);

    // NOTE: 結果によってエラーを返す
    if (size == 0)
    {
        printf("Error: Failed to read file\n");
        return ERROR_FLAG;
    }

    return size;
}

/**
 * JSONを解析
 * @param json - 対象のjson
 * @param key_value - JsonPair構造体のアドレス この構造体に解析した値が格納される
 * @param pairs_count - `key:value`のペアを最大いくつ生成するか
 * @return 解析したペアの数
 */
int parseJson(char *json, KeyValue *key_value, int pairs_count)
{
    // NOTE: 引数jsonに渡された文字列を `{` , `}` , `:` , ` ` のいずれかで分割
    char *token = strtok(json, "{},: ");
    char *key, *value;
    int i = 0;

    while (token != NULL && i < pairs_count)
    {
        // NOTE: 最初のトークンをkeyとする
        key = token;
        token = strtok(NULL, "{},: ");

        // NOTE: 次のトークンをvalueとする
        if (token != NULL)
        {
            value = token;
            token = strtok(NULL, "{},: ");
        }
        else
        {
            value = NULL;
        }

        // NOTE: `key`の値をpairsのkeyに格納
        copyStringSafely(key_value[i].key, key, sizeof(key_value[i].key));

        if (value != NULL)
        {
            // NOTE: `value`の値をpairsのvalueに格納
            copyStringSafely(key_value[i].value, value, sizeof(key_value[i].value));
        }
        else
        {
            key_value[i].value[0] = '\0';
        }

        i++;
    }

    return i;
}

/**
 * ファイルの読み込みを行う
 * @param body ボディを格納するバッファへのアドレス
 * @param file_path リクエストターゲットに対するファイルへのパス
 * @return ステータスコード (ファイルがない場合は404)
 */
int readFile(char *body, const char *file_path)
{
    // NOTE: ファイルサイズを取得
    int file_size = getFileSize(file_path);
    if (isError(file_size))
    {
        // NOTE: ファイルサイズが0やファイルが存在しない場合は404を返す
        printf("Error get file size: %s\n", file_path);
        return HTTP_STATUS_NOT_FOUND;
    }

    // NOTE: ファイルを読み込み
    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        printf("Error opening file: %s, error: %s\n", file_path, strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }

    // NOTE: ファイルを読み込んで、bodyに格納
    fread(body, DATA_BLOCK_SIZE_FOR_READ, file_size, file);
    fclose(file);

    return HTTP_STATUS_OK;
}

void copyStringSafely(char *destination, char *source, size_t destination_size)
{
    strncpy(destination, source, destination_size - 1);
    destination[destination_size - 1] = '\0';
}

int isError(int target)
{
    if (target == ERROR_FLAG)
    {
        return 1;
    }
    return 0;
}

int isMatchStr(const char *str1, const char *str2)
{
    if (strcmp(str1, str2) == 0)
    {
        return 1;
    }
    return 0;
}