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
    Route routes[] = {
        {"GET", "/", "text/html", "/index.html", "", NULL},
        {"GET", "/user/:id", "text/html", "/index.html", "", requestHandler},
        {"GET", "/user/:id/textbook/:textbookId", "text/html", "/index.html", "", requestHandler},
        {"POST", "/test", "text/html", "/test.html", "", requestHandler},
        {"POST", "/plain", "text/plain", "", "hello world!", requestHandler},
    };

    int res = connectHttpServer(routes, sizeof(routes) / sizeof(routes[0]));
    return res;
}

void requestHandler(const HttpRequest *const request, const HttpResponse *const response)
{
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

2. 以下のコマンドで、サーバーを起動させます(ここで、サーバーは待機状態になります。)

```bash
make runServer
```

3. 任意のコマンドで、サーバーにリクエストを送ります。以下はその例です

```bash
# GETリクエスト
curl -i http://127.0.0.1:8080/

# POSTリクエスト
curl -X POST -H "Content-Type: application/json" -d '{"name":"johnDoe"}' http://127.0.0.1:8080/user/
```
