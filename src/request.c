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

    return recv_size;
}

/**
 * リクエストメッセージを解析する
 * @param method メソッドを格納するバッファへのアドレス
 * @param req_target リクエストターゲットを格納するバッファへのアドレス
 * @param request_message 解析するリクエストメッセージが格納されたバッファへのアドレス
 * @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
 */
int parseRequestMessage(char *method, char *req_target, char *request_message)
{
    char *line;
    char *tmp_method;
    char *tmp_target;

    // NOTE: リクエストメッセージの1行目を取得
    line = strtok(request_message, "\n");
    // NOTE: " "までの文字を取得
    tmp_method = strtok(line, " ");
    if (tmp_method == NULL)
    {
        printf("get method error\n");
        return -1;
    }
    // NOTE: tmp_method を methodにコピー
    strcpy(method, tmp_method);

    // NOTE: 次の" "までの文字列を取得
    tmp_target = strtok(NULL, " ");
    if (tmp_target == NULL)
    {
        printf("get target error\n");
        return -1;
    }
    // NOTE: tmp_target を targetにコピー
    strcpy(req_target, tmp_target);

    return 0;
}

/**
 * リクエストに対する処理を行う(今回はGETのみ)
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
