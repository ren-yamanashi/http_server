#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"
#include "response.h"
#include "request.h"
#include "io.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define SIZE (5 * 1024)

// NOTE: `strcmp` 二つの文字列を比較。一致する場合は0を、一致しない場合は0以外の値を返す
// NOTE: `strtok` 文字列を特定の区切り文字に基づいてトークン(部分文字列)に分割　第一引数にNULLを渡すことで、前回区切った文字列の次のトークンを取得

/**
 * 受信した文字列を表示
 * @param message メッセージを格納するバッファへのアドレス
 * @param size メッセージのバイト数
 */
void showMessage(char *message, unsigned int size)
{
    unsigned int i;
    printf("Show Message\n\n");

    for (i = 0; i < size; i++)
    {
        putchar(message[i]);
    }
    printf("\n\n");
}

/**
 * HTTPサーバーの処理を行う関数
 * @param sock: 接続済みのソケット
 * @return 0
 */
int httpServer(int sock)
{
    int request_size, response_size;
    char request_message[SIZE];
    char response_message[SIZE];
    char method[SIZE];
    char target[SIZE];
    char header_field[SIZE];
    char body[SIZE];
    int status;
    unsigned int file_size;

    while (1)
    {
        // NOTE: リクエストメッセージを受信
        request_size = recvRequestMessage(sock, request_message, SIZE);
        if (request_size == -1)
        {
            printf("recvRequestMessage error\n");
            break;
        }

        if (request_size == 0)
        {
            // NOTE: 受信サイズが0の場合は相手が接続を閉じていると判断
            printf("connection ended\n");
            break;
        }

        showMessage(request_message, request_size);

        // NOTE: 受信した文字列を解析してメソッドやリクエストターゲットを取得
        if (parseRequestMessage(method, target, request_message) == -1)
        {
            printf("parseRequestMessage error\n");
            break;
        }
        
        // NOTE: requestMethodが受信可能なものか判別
        if (checkRequestMethod(method) != 0)
        {
            status = 404;
        }
        else
        {
            if (strcmp(target, "/") == 0)
            {
                // NOTE: `/`が指定されたときは`/index.html`に置き換える
                strcpy(target, "/index.html");
            }
            else
            {
                // NOTE: とりあえず、`~/hoge`リクエストに対して、`hoge.html`ファイルを返す
                strcat(target, ".html");
            }
            status = processingRequest(body, &target[1]);
        }

        file_size = getFileSize(&target[1]);
        response_size = createResponseMessage(response_message, status, header_field, body, file_size);
        if (response_size == -1)
        {
            printf("createResponseMessage error\n");
            break;
        }

        // NOTE: 送信するメッセージを表示
        showMessage(response_message, response_size);

        // NOTE: レスポンスメッセージを送信する
        sendResponseMessage(sock, response_message, response_size);
    }
    return 0;
}

int main(void)
{
    int w_addr, c_sock, DEFAULT_PROTOCOL = 0;
    struct sockaddr_in a_addr;

    /**
     *  ソケットを作成
     *  @param AF_INET プロトコルファミリー(アドレスファミリー)を指定
     *  @param SOCK_STREAM　ソケットのタイプを指定
     *  @param DEFAULT_PROTOCOL　使用するプロトコルを指定
     *  @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
     */
    w_addr = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (w_addr == -1)
    {
        printf("socket error\n");
        return -1;
    }

    // NOTE: 構造体を全て0にセット
    memset(&a_addr, 0, sizeof(struct sockaddr_in));

    /** サーバーのIPアドレスとボート番号の情報を設定 */
    // NOTE: アドレスファミリーを指定
    a_addr.sin_family = AF_INET;
    // NOTE: 使用するポート番号を指定
    a_addr.sin_port = htons((unsigned short)SERVER_PORT);
    // NOTE: 使用するIPアドレスを指定
    a_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    /**
     *  ソケットを特定のネットワークアドレス（IPアドレスとポート番号の組）に紐付ける
     *  @param w_addr ソケット
     *  @param `(const struct sockaddr *)&a_addr` ソケットに割り当てるアドレスやポート番号の情報 sockaddr_in構造体のインスタンスで、IPアドレスとポート番号を含む
     *  @param `sizeof(a_addr)` addrのサイズ(バイト数)
     *  @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
     */
    if (bind(w_addr, (const struct sockaddr *)&a_addr, sizeof(a_addr)) == -1)
    {
        printf("bind error\n");
        close(w_addr);
        return -1;
    }

    /**
     *  ソケットを接続待ちに設定
     *  @param sockfd 接続を待つソケット
     *  @param backlog 接続要求を保持する数
     *  @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
     */
    if (listen(w_addr, 3) == -1)
    {
        printf("listen error\n");
        close(w_addr);
        return -1;
    }

    while (1)
    {
        printf("Waiting connect...\n");

        /**
         *  接続を受け付ける
         *  @param sockfd 接続待ちの状態になっているソケット
         *  @param addr 接続先の情報へのポインタ
         *  @param addrlen addrのサイズ(バイト数)へのポインタ
         *  @return 接続が確立されたソケット
         */
        c_sock = accept(w_addr, NULL, NULL);
        if (c_sock == -1)
        {
            printf("accept error\n");
            close(w_addr);
            return -1;
        }
        printf("Connected!!\n");

        // NOTE: 接続済みのソケットでデータのやり取り
        httpServer(c_sock);

        // NOTE: ソケット通信をクローズ
        close(c_sock);

        // NOTE: 次の接続要求を受け付ける
    }
    // NOTE: 接続待ちのソケットをクローズ
    close(w_addr);
    return 0;
}