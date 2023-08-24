# HttpServer の処理

<details><summary>1. ソケットを作成</summary>

HTTP 通信を行う際には、まずソケットを作成します。それを他のアプリやプログラムのソケットに接続してデータのやり取りを行います。
※ソケットとは通信端点のことで、アプリとアプリ(プログラムとプログラム)の通信の出入り口のことです。

```c
/**
 *  @param domain プロトコルファミリー(アドレスファミリー)を指定
 *  @param type　ソケットのタイプを指定
 *  @param protocol　使用するプロトコルを指定
 *  @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
 */
init socket(int domain, int type, int protocol);

int waiting_sock_addr = socket(AF_INET, SOCK_STREAM, 0);
```

</details>

<details><summary>2. ソケットを特定の IP アドレスとポート番号の組に紐付け</summary>

IP アドレス・ポート番号・アドレスファミリーを指定し、その情報を 1 で作成したソケットに紐付け(関連付け)します。
アドレスファミリーとは、アドレスの種類です。アドレスの種類を指定して、どのようなプロトコルやアドレス形式を使用して通信を行うのかを決めます。(IPv4 を使うとか、IPv6 を使うとか、そういった約束事の種類を決めます)
ポート番号や IP アドレスは、アドレスの格納方法が異なるシステム間でもネットワーク通信を行えるように、データの変換を行います

```c
struct sockaddr_in sock_addr_info;

// NOTE: アドレスファミリーを指定 (IPv4)
sock_addr_info.sin_family = AF_INET;
// NOTE: 使用するポート番号を指定
sock_addr_info.sin_port = htons((unsigned short)8080);
// NOTE: 使用するIPアドレスを指定
sock_addr_info.sin_addr.s_addr = inet_addr("127.0.0.1");
```

```c
/**
 *  @param sockfd ソケット
 *  @param addr　ソケットに割り当てるアドレスやポート番号の情報
 *  @param addrlen addrのサイズ(バイト数)
 *  @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
 */
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

bind(waiting_sock_addr, (const struct sockaddr *)&sock_addr_info, sizeof(sock_addr_info)))
```

</details>

<details><summary>3. ソケットを接続待ちに設定</summary>

ソケットを接続待ち状態にし、クライアントからの接続要求を可能にします。
実際には、接続要求を溜めるためのキューを作成します。その為、ここでは接続要求を保持する数を設定します。
例えば、サーバー側がリクエスト A の処理を行なっている間に、別のリクエスト B が来たとします。この時、リクエスト B は処理待ち状態に入ります(キューに登録されます)。そして、サーバー側がリクエスト A の処理を終えた時点でキューから取り出され、リクエスト B の処理に入ります。キューなので FIFO(先入先出)です
接続要求を保持する数というのは、この時の処理待ち状態にできるリクエストの数です。

```c
/**
 *  @param sockfd 接続を待つソケット
 *  @param backlog 接続要求を保持する数
 *  @return 成功したかのフラグ 成功時に0、エラー時に-1を返す
 */
int listen(int sockfd, int backlog);

listen(waiting_sock_addr, 3)
```

</details>

<details><summary>4. 接続を受け付ける</summary>

ここでは、listen で溜めた接続要求から接続要求を取得し、その接続要求を受け付けて実際のデータのやり取りを開始します。

```c
/**
 *  @param sockfd 接続待ちの状態になっているソケット
 *  @param addr　接続先の情報へのポインタ
 *  @param addrlen　addrのサイズ(バイト数)へのポインタ
 *  @return 接続が確立されたソケット
 */
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int connected_sock_addr = accept(waiting_sock_addr, NULL, NULL);

```

listen と accept の関係がちょっとややこしいのですが、キューで例えると以下のような感じです

1. `listen`で接続要求を溜めておくためのキューを作成します
2. クライアントからの接続要求が来ると、接続要求はキューに溜められます。
3. `accept` ではこのキューから接続要求を取得して、その接続要求を受け付けてデータのやり取りを開始します。
   これにより、サーバーは接続要求が来たタイミングではなく、サーバー自身のタイミングが良い時(他のクライアントとデータのやり取りをしていない時)に接続を受け付け、やり取りを行うことが可能になります。
   ※`socket` によって作成されたソケットのアドレスと、`accept` の戻り値のソケットのアドレスは別です。`accept` 関数はクライアントからの接続要求を受け付けて、そのクライアントと通信を行うための新しいソケットのアドレスを返します。
   → 元のソケット(`socket` 関数で作成したもの)は引き続き接続要求の受付に使用されます

</details>

<details><summary>5. リクエストを受信</summary>

     クライアントからのリクエストを受信します。

```c

/**
 *  @param sockfd 接続済みのソケット
 *  @param buf 受信データを格納するバッファのアドレス
 *  @param len bufのサイズ(バイト数)
 *  @param flags 受信時の動作の詳細設定
 *  @return 実際に接続先から受信したデータのバイト数
 */
int recv(int sockfd, const void *buf, size_t len, int flags);

char request_message[1024];
unsigned int buf_size = 1024;
int recv_size = recv(connected_sock_addr, request_message, buf_size, 0);
```

</details>

<details><summary>6. リクエスト内容に応じて、レスポンスをクライアントに送信</summary>

     リクエストを受信したら、その内容に応じてレスポンスを作成します。
     例えば、リクエストターゲットが`/` なら`index.html` ファイルを読み取って、その内容をレスポンスボディにするとかです。

```c
/**
 *  @param sockfd 接続済みのソケット
 *  @param buf 送信するデータへのポインタ
 *  @param len 送信するデータのサイズ(バイト数)
 *  @param flags 送信時の動作の詳細設定
 *  @return 実際に接続先に送信したデータのバイト数
 */
ssize_t send(int sockfd, const void *buf, size_t len, int flags);

char response_message[1024];
int response_size;
//
//　レスポンスを作成する処理
//
send(connected_sock_addr, response_message, response_size, 0);
```

</details>

<details><summary>7. 5 のソケットを閉じ、次の接続要求を受け付ける( 5 に戻る)</summary>

     クライアントに対してレスポンスを送信後、接続を閉じます。
     ここで閉じるソケットは、接続が確立されたソケット(`accept`関数の戻り値のソケット)なので、まだサーバーは待機状態です。
     待機状態なので、クライアントからの接続要求がきた場合はまた新しい接続が確立され、同様に処理が繰り返されます。

```c
/**
 *  @param fd ソケットの識別子
 */
int close(int fd);

close(connected_sock_addr);
```

</details>

<details><summary>8. 1 のソケットを閉じる</summary>

     データのやり取りが一通り終わった後に、接続要求の受付に使用されているソケットを閉じます。
     ここまでで、HttpServer の処理は終わりです。

```c
/**
 *  @param fd ソケットの識別子
int close(int fd);

close(waiting_sock_addr);
```

</details>

<br />
<br />

# C 言語の標準ライブラリ関数

- strcat
  文字列の連結を行う。
  第二引数で指定した文字列を第一引数で指定した文字列の終端に追加

- strcmp
  二つの文字列を比較。一致する場合は 0 を、一致しない場合は 0 以外の値を返す

- strtok
  文字列を特定の区切り文字に基づいてトークン(部分文字列)に分割
  第一引数に NULL を渡すことで、前回区切った文字列の次のトークンを取得

- sprintf
  printf と同じフォーマット規則を使いながら指定したバッファに文字列を出力

```c
// example

#include <stdio.h>

int main() {
    char buffer[50];
    int a = 10;
    float b = 20.5;

    sprintf(buffer, "Integer = %d, Float = %.2f", a, b);
    printf("Formatted String: %s\n", buffer);

    return 0;
}
```

- strncpy
  一つの文字列から別の文字列へ指定された数の文字をコピーする
  n 文字をコピーした時点でソース文字列の終端に達していない場合、文字列には終端を意味する null 文字が追加されない。
  なので、終端に`\0`を追加する必要がある。

```c
const char *message = "Hello, world!";
char body[1024];
strncpy(body, message, sizeof(body) - 1);
body[sizeof(body) - 1] = '\0';
```
