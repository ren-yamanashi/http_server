#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "request.h"
#include "io.h"

/**
 * リクエストメソッドが受信可能なものか判別
 * @param req_method リクエストメソッド
 * @return 可能な場合は0それ以外は-1
 */
int checkRequestMethod(const char *req_method)
{
    if (strcmp(req_method, "GET") == 0 || strcmp(req_method, "POST") == 0)
    {
        return 0;
    }
    return -1;
}

/**
 * リクエストメッセージを受信する
 * @param sock 接続済みのソケット
 * @param request_message リクエストメッセージを格納するバッファへのアドレス
 * @param buf_size リクエストメッセージを格納するバッファのサイズ
 * @return 実際に接続先から受信したデータのバイト数
 */
int recvRequestMessage(int sock, char *request_message, unsigned int buf_size)
{
    // NOTE: 受信時、送信時の動作の詳細設定: 今回は特別なフラグを設定しないので`0`とする
    int RECV_FLAG = 0;
    int recv_size;

    /**
     *  @param sockfd 接続済みのソケット
     *  @param request_message 受信データを格納するバッファのアドレス
     *  @param buf_size bufのサイズ(バイト数)
     *  @param RECV_FLAG 受信時の動作の詳細設定
     *  @return 実際に接続先から受信したデータのバイト数
     */
    recv_size = recv(sock, request_message, buf_size, RECV_FLAG);

    // バッファの現在の終端をNULL文字で終了
    request_message[recv_size] = '\0';

    return recv_size;
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
    char *header, *header_save, *header_value;
    char *req_method = NULL;
    char *req_target = NULL;
    char *version = NULL;

    // NOTE: ヘッダーとボディを分離
    char *headers_end = strstr(request_message, "\r\n\r\n");

    if (!headers_end)
    {
        printf("Could not find end of headers\n");
        return -1;
    }

    // ヘッダー部分をnullで終了し、ボディを「\r\n\r\n」の後に開始するように設定
    *headers_end = '\0';
    char *body_start = headers_end + 4;

    // NOTE: リクエストメッセージの1行目を取得
    line = strtok_r(request_message, "\r\n", &line_save);

    if (line == NULL)
    {
        printf("Could not get request\n");
        return -1;
    }

    // NOTE: 1行目の情報を取得
    req_method = strtok_r(line, " ", &header_save);
    req_target = strtok_r(NULL, " ", &header_save);
    version = strtok_r(NULL, " ", &header_save);

    if (req_method == NULL || req_target == NULL || version == NULL)
    {
        printf("Could not parse the request line\n");
        return -1;
    }

    // NOTE: 1行目の情報を構造体に格納
    strncpy(request->method, req_method, sizeof(request->method) - 1);
    strncpy(request->target, req_target, sizeof(request->target) - 1);
    strncpy(request->version, version, sizeof(request->version) - 1);

    // NOTE: 続く行を取得
    line = strtok_r(NULL, "\r\n", &line_save);

    while (line)
    {
        header = strtok_r(line, ":", &header_save);
        header_value = strtok_r(NULL, "", &header_save);
        if (header && header_value && strcmp(header, "Content-Type") == 0)
        {
            // NOTE: コンテンツタイプを取得
            // NOTE: `:`の後のスペースをスキップ
            header_value++;
            strncpy(request->contentType, header_value, sizeof(request->contentType) - 1);
            request->contentType[sizeof(request->contentType) - 1] = '\0';
        }

        // NOTE: 行の取得を繰り返す
        line = strtok_r(NULL, "\r\n", &line_save);
    }

    // NOTE: ボディの取得
    snprintf(request->body, sizeof(request->body), "%s", body_start);

    // リクエストの情報を表示
    printf("Parsed the request!!\n");
    printf("Method: %s\r\n", request->method);
    printf("Target: %s\r\n", request->target);
    printf("Http Version: %s\r\n", request->version);
    printf("Content-Type: %s\r\n", request->contentType);
    printf("body: %s\r\n", request->body);

    parseRequestBody(request);

    return 0;
}

/**
 * リクエストボディを解析
 * jsonの場合は、objectに変換
 * plainTextの場合は、何もしない
 * @param request HttpRequest構造体のアドレス
 * @return 成功した場合は0 それ以外は-1
 *
 */
int parseRequestBody(HttpRequest *request)
{

    if (strcmp(request->contentType, "application/json") == 0)
    {
        KeyValue keyValue[5];
        parseJson(request->body, keyValue, 5);
        for (int i = 0; i < 5 && keyValue[i].key[0] != '\0'; i++)
        {
            printf("Key: %s, Value: %s\n", keyValue[i].key, keyValue[i].value);
        }
        return 0;
    }
    else if (strcmp(request->contentType, "text/plain") == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
 * リクエストに対する処理を行う
 * @param body ボディを格納するバッファへのアドレス
 * @param file_path リクエストターゲットに対するファイルへのパス
 * @return ステータスコード (ファイルがない場合は404)
 */
int processingRequest(char *body, char *file_path)
{
    FILE *file;
    int file_size, DATA_BLOCK_SIZE_FOR_READ = 1;

    // NOTE: ファイルサイズを取得
    file_size = getFileSize(file_path);
    if (file_size == 0)
    {
        // NOTE: ファイルサイズが0やファイルが存在しない場合は404を返す
        printf("getFileSize error\n");
        return 404;
    }

    // NOTE: ファイルを読み込み
    file = fopen(file_path, "rb");
    if (file == NULL)
    {
        printf("Error opening file: %s\n", file_path);
        return 404;
    }
    // NOTE: ファイルを読み込んで、bodyに格納
    fread(body, DATA_BLOCK_SIZE_FOR_READ, file_size, file);
    fclose(file);

    return 200;
}
