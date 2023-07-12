#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "io.h"

#define SIZE (5 * 1024)
/**
 * ファイルサイズの取得
 * @param path ファイルパスを指す文字列
 * @return pathを元に読み込んだファイルのサイズ
 */
unsigned int getFileSize(const char *path)
{
    int size, read_size;
    char read_buf[SIZE];
    FILE *f;

    // NOTE: ファイルを開く
    f = fopen(path, "rb");
    if (f == NULL)
    {
        return 0;
    }

    size = 0;
    do
    {
        // NOTE: ファイルから指定した数・サイズのデータを読み込む
        read_size = fread(read_buf, 1, SIZE, f);
        size += read_size;
    } while (read_size != 0);

    fclose(f);

    return size;
}

/**
 * JSONを解析
 * @param json 対象のjson
 * @param key_value JsonPair構造体のアドレス この構造体に解析した値が格納される
 * @param pairs_count `key:value`のペアを最大いくつ生成するか
 * @return 解析したペアの数
 */
int parseJson(char *json, KeyValue *key_value, int pairs_count)
{
    char *key, *value;
    // NOTE: 引数jsonに渡された文字列を `{` , `}` , `:` , ` ` のいずれかで分割
    char *token = strtok(json, "{},: ");
    int i = 0;

    while (token != NULL && i < pairs_count)
    {
        // NOTE: 最初のトークンをkeyとする
        key = token;
        token = strtok(NULL, "{},: ");

        // NOTE: 次のトークンをvalueとする
        if (token != NULL)
        {
            value = token;
            token = strtok(NULL, "{},: ");
        }
        else
        {
            value = NULL;
        }

        // NOTE: `key`の値をpairsのkeyに格納
        strncpy(key_value[i].key, key, sizeof(key_value[i].key) - 1);
        key_value[i].key[sizeof(key_value[i].key) - 1] = '\0';

        if (value != NULL)
        {
            // NOTE: `value`の値をpairsのvalueに格納
            strncpy(key_value[i].value, value, sizeof(key_value[i].value) - 1);
            key_value[i].value[sizeof(key_value[i].value) - 1] = '\0';
        }
        else
        {
            key_value[i].value[0] = '\0';
        }

        i++;
    }

    return i;
}

/**
 * ファイルの読み込みを行う
 * @param body ボディを格納するバッファへのアドレス
 * @param file_path リクエストターゲットに対するファイルへのパス
 * @return ステータスコード (ファイルがない場合は404)
 */
int readFile(char *body, const char *file_path)
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
