#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "response.h"

/**
 * レスポンスメッセージを作成する
 * @param response_message レスポンスメッセージを格納するバッファへのアドレス
 * @param status ステータスコード
 * @param header ヘッダーフィールドを格納したバッファへのアドレス
 * @param body ボディを格納したバッファへのアドレス
 * @param body_size ボディのサイズ
 * @return レスポンスメッセージのデータサイズ(バイト長)
 */
int createResponseMessage(char *response_message, int status, char *header, char *body, unsigned int body_size)
{
    unsigned int no_body_len;
    unsigned int body_len;
    char content_length[50];
    response_message[0] = '\0';

    sprintf(content_length, "Content-Length: %u\r\nContent-Type: text/html\r\n", body_size);
    strcat(header, content_length);
    if (status == 200)
    {
        // NOTE: レスポンス行とヘッダーフィールドの文字列を作成
        sprintf(response_message, "HTTP/1.1 200 OK\r\n%s\r\n", header);

        no_body_len = strlen(response_message);
        body_len = body_size;

        // NOTE: ヘッダーフィールドの後ろにボディをコピー
        memcpy(&response_message[no_body_len], body, body_len);
    }
    else if (status == 404)
    {
        // NOTE: レスポンス行とヘッダーフィールドの文字列を作成
        sprintf(response_message, "HTTP/1.1 404 Not Found\r\n%s\r\n", header);

        no_body_len = strlen(response_message);
        body_len = 0;
    }
    else
    {
        // NOTE: statusコードをプログラムがサポートしていない場合
        printf("Not support status(%d)\n", status);
        return -1;
    }

    return no_body_len + body_len;
}

/**
 * レスポンスメッセージを送信する
 * @param sock: 接続済みのソケット
 * @param response_message: 送信するレスポンスメッセージへのアドレス
 * @param message_size: 送信するメッセージのサイズ
 * @return 送信したデータサイズ(バイト長)
 */
int sendResponseMessage(int sock, char *response_message, unsigned int message_size)
{
    // NOTE: 受信時、送信時の動作の詳細設定: 今回は特別なフラグを設定しないので`0`とする
    int SEND_FLAG = 0;
    int send_size;

    /**
     *  データを送信する
     *  @param sock 接続済みのソケット
     *  @param response_message 送信するデータへのポインタ
     *  @param message_size 送信するデータのサイズ(バイト数)
     *  @param SEND_FLAG 送信時の動作の詳細設定
     *  @return 実際に接続先に送信したデータのバイト数
     */
    send_size = send(sock, response_message, message_size, SEND_FLAG);

    return send_size;
}