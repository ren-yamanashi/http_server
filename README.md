# Usage

1. `main.c`ファイルで、以下のようにルートを設定します

※ `main.c`ファイルの関数は、必ず`main.h`ファイルに追加してください。

```c
int main(void)
{
    /**
     * Route構造体は、左からそれぞれ以下のような意味を持ちます
     * - HTTP リクエストメソッド
     * - リクエストパス
     * - レスポンスヘッダー`Contents-Type`の値 (`text/html`と`text/plain`のみ設定可能です)
     * - レスポンスファイル (`Contents-Type`が`text/html`の場合にのみ指定したファイルが出力されます)
     * - レスポンスメッセージ (`Contents-Type`が`text/plain`の場合にのみ指定した文字列が出力されます)
     * - ハンドラー関数 HttpRequest と HttpResponse を受け取って、実行する処理を指定できます。リクエストが正常な場合に、この処理は実行されます
     */
    Route routes[6];
    createRoute(&routes[0], HTTP_METHOD_GET, "/", CONTENT_TYPE_HTML, "/index.html", "", NULL);
    createRoute(&routes[1], HTTP_METHOD_GET, "/user/:id", CONTENT_TYPE_HTML, "/index.html", "", requestHandler);
    createRoute(&routes[2], HTTP_METHOD_GET, "/user/:id/textbook/:textbookId", CONTENT_TYPE_HTML, "/index.html", "", requestHandler);
    createRoute(&routes[3], HTTP_METHOD_POST, "/test", CONTENT_TYPE_HTML, "/index.html", "", requestHandler);
    createRoute(&routes[4], HTTP_METHOD_POST, "/plain", CONTENT_TYPE_PLAIN, "", "hello world!", requestHandler);
    createRoute(&routes[5], HTTP_METHOD_DELETE, "/data/:id/delete", CONTENT_TYPE_PLAIN, "", "delete data", requestHandler);

    int res = runServer(routes, sizeof(routes) / sizeof(routes[0]));
    if (res < 0)
    {
        fprintf(stderr, "Failed to connect to the HTTP server. Error code: %d\n", res);
        return 1;
    }
    return 0;
}

void requestHandler(const HttpRequest *const request, const HttpResponse *const response)
{
    // この中身は好きに変更する
    for (int i = 0; i < request->parsed_kv_count; i++)
    {
        printf("Request Body: {%s: %s}\n", request->parsed_kv[i].key, request->parsed_kv[i].value);
    }
    for (int i = 0; i < request->param_kv_count; i++)
    {
        printf("Request param: {%s: %s}\n", request->param_kv[i].key, request->param_kv[i].value);
    }
}
```

2. サーバーを起動させます(ここで、サーバーは待機状態になります。)

```bash
make runServer
```

3. サーバーにリクエストを送ります。以下はその例です

```bash
# GETリクエスト
curl -i http://127.0.0.1:8080/

# POSTリクエスト
curl -X POST -H "Content-Type: application/json" -d '{"name":"johnDoe"}' http://127.0.0.1:8080/user/

# DELETEリクエスト
curl -X DELETE http://127.0.0.1:8080/data/2/delete
```

# Node FFI

C 言語で作成した関数を Node.js 環境で実行できます

1. サーバーを起動させます(ここで、サーバーは待機状態になります。)

```bash
make runNodeFFI
```

2. サーバーにリクエストを送ります。

```bash
curl -i http://127.0.0.1:8080/
```
