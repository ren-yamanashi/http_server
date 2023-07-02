#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define SIZE (5 * 1024)

int httpServer(int);
int recvRequestMessage(int, char *, unsigned int);
int parseRequestMessage(char *, char *, char *);
int getProcessing(char *, char *);
int createResponseMessage(char *, int, char *, char *, unsigned int);
int sendResponseMessage(int, char *, unsigned int);
unsigned int getFileSize(const char *);

// NOTE: ファイルサイズを取得する
unsigned int getFileSize(const char *path)
{
    int size, read_size;
    char read_buf[SIZE];
    FILE *f;

    f = fopen(path, "rb");
    if (f == NULL)
        return 0;

    size = 0;
    do
    {
        read_size = fread(read_buf, 1, SIZE, f);
        size += read_size;
    } while (read_size != 0);

    fclose(f);
    return size;
}

/**
 * リクエストメッセージを受信する
 * @param sock: 接続済みのソケット
 * @param request_message: リクエストメッセージを格納するバッファへのアドレス
 * @param buf_size: リクエストメッセージを格納するバッファのサイズ
 * @return 受信したデータのサイズ(バイト長)
 */
int recvRequestMessage(int sock, char *request_message, unsigned int buf_size)
{
    // NOTE: 受信時、送信時の動作の詳細設定: 今回は特別なフラグを設定しないので`0`とする
    int RECT_FLAG = 0, SEND_FLAG = 0;
    int recv_size;

    recv_size = recv(sock, request_message, buf_size, RECT_FLAG);

    return recv_size;
}

/**
 * リクエストメッセージを解析する
 * @param method: メソッドを格納するバッファへのアドレス
 * @param target: リクエストターゲットを格納するバッファへのアドレス
 * @param request_message: 解析するリクエストメッセージが格納されたバッファへのアドレス
 * @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
 */
int parseRequestMessage(char *method, char *target, char *request_message)
{
    char *line;
    char *tmp_method;
    char *tmp_target;

    // NOTE: リクエストメッセージの1行目の未取得
    line = strtok(request_message, "\n");

    // NOTE: " "までの文字を取得しmethodにコピー
    tmp_method = strtok(line, " ");
    if (tmp_method == NULL)
    {
        printf("get method error\n");
        return -1;
    }
    strcpy(method, tmp_method);

    // NOTE: 次の" "までの文字列を取得し、targetにコピー
    tmp_target = strtok(NULL, " ");
    if (tmp_target == NULL)
    {
        printf("get target error\n");
        return -1;
    }

    strcpy(target, tmp_target);
    return 0;
}

/**
 * リクエストに対する処理を行う(今回はGETのみ)
 * @param body: ボディを格納するバッファへのアドレス
 * @param file_path: リクエストターゲットに対するファイルへのパス
 * @return ステータスコード (ファイルがない場合は404)
 */
int getProcessing(char *body, char *file_path)
{
    FILE *f;
    int file_size;

    // NOTE: ファイルサイズを取得
    file_size = getFileSize(file_path);
    if (file_size == 0)
    {
        // NOTE: ファイルサイズが0やファイルが存在しない場合は404を返す
        printf("getFileSize error\n");
        return 404;
    }

    // NOTE: ファイルを読み込んでボディとする
    f = fopen(file_path, "r");
    fread(body, 1, file_size, f);
    fclose(f);

    return 200;
}

/**
 * レスポンスメッセージを作成する
 * @param response_message: レスポンスメッセージを格納するバッファへのアドレス
 * @param status: ステータスコード
 * @param header: ヘッダーフィールドを格納したバッファへのアドレス
 * @param body: ボディを格納したバッファへのアドレス
 * @param body_size: ボディのサイズ
 * @return レスポンスメッセージのデータサイズ(バイト長)
 */
int createResponseMessage(char *response_message, int status, char *header, char *body, unsigned int body_size)
{
    unsigned int no_body_len;
    unsigned int body_len;

    response_message[0] = "\0";

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

    send_size = send(sock, response_message, message_size, SEND_FLAG);

    return send_size;
}

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

        // NOTE: 受信した文字列を表示
        showMessage(request_message, request_size);

        // NOTE: 受信した文字列を解析してメソッドやリクエストターゲットを取得
        if (parseRequestMessage(method, target, request_message) == -1)
        {
            printf("parseRequestMessage error\n");
            break;
        }

        // NOTE: メソッドがGET以外はステータスコードを404にする
        if (strcmp(method, "GET") != 0)
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
            // NOTE: GETの応答をするために必要な処理を行う
            status = getProcessing(body, &target[1]);
        }

        // NOTE: ヘッダーフィールド作成(今回はContent-Lengthのみ)
        file_size = getFileSize(&target[1]);
        sprintf(header_field, "Content-Length: %u\r\n", file_size);

        // NOTE: レスポンスメッセージを作成
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

    // NOTE: ソケットを作成
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

    // NOTE: ソケットを特定のネットワークアドレス（IPアドレスとポート番号の組）に紐付ける
    /**
     *  @param w_addr ソケット
     *  @param `(const struct sockaddr *)&a_addr`　ソケットに割り当てるアドレスやポート番号の情報 sockaddr_in構造体のインスタンスで、IPアドレスとポート番号を含む
     *  @param `sizeof(a_addr)` addrのサイズ(バイト数)
     *  @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
     */
    if (bind(w_addr, (const struct sockaddr *)&a_addr, sizeof(a_addr)) == -1)
    {
        printf("bind error\n");
        close(w_addr);
        return -1;
    }

    // NOTE: ソケットを接続待ちに設定
    if (listen(w_addr, 3) == -1)
    {
        printf("listen error\n");
        close(w_addr);
        return -1;
    }

    while (1)
    {
        // NOTE: 接続を受け付ける
        printf("Waiting connect...\n");
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
