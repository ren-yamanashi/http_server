#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "response.h"
#include "constance.h"

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
