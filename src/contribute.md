### C言語の標準ライブラリ関数
- strncpy
一つの文字列から別の文字列へ指定された数の文字をコピーする
n文字をコピーした時点でソース文字列の終端に達していない場合、文字列には終端を意味するnull文字が追加されない。
なので、終端に`\0`を追加する必要がある。
```c
/**
 * 
 * @param dest コピー先の文字列
 * @param src コピー元の文字列
 * @param n コピーする最大文字数
 * strncpy(char *dest, const char *src, size_t n);
 *
 * // example
 * request->contentType[sizeof(request->contentType) - 1] = "\0";
 */
```

- strcat
文字列の連結を行う。
第二引数で指定した文字列を第一引数で指定した文字列の終端に追加

- strcmp
二つの文字列を比較。一致する場合は0を、一致しない場合は0以外の値を返す

- strtok
文字列を特定の区切り文字に基づいてトークン(部分文字列)に分割
第一引数にNULLを渡すことで、前回区切った文字列の次のトークンを取得

- sprintf
printfと同じフォーマット規則を使いながら指定したバッファに文字列を出力
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