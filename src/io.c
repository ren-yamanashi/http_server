#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "io.h"
#include "helper.h"
#include "constance.h"

/**
 * ファイルサイズの取得
 * @param path - ファイルパスを指す文字列
 * @return pathを元に読み込んだファイルのサイズ
 */
int getFileSize(const char *path)
{
    int size = 0, read_size = 0;
    char read_buf[SIZE];
    FILE *file;

    // NOTE: ファイルを開く
    file = fopen(path, "rb");
    if (file == NULL)
    {
        printf("Error: opening file: %s, error: %s\n", path, strerror(errno));
        return ERROR_FLAG;
    }

    // NOTE: ファイルから指定した数・サイズのデータを読み込む
    do
    {
        read_size = fread(read_buf, 1, SIZE, file);
        size += read_size;
    } while (read_size != 0);

    // NOTE: ファイルを閉じる
    fclose(file);

    // NOTE: 結果によってエラーを返す
    if (size == 0)
    {
        printf("Error: Failed to read file\n");
        return ERROR_FLAG;
    }

    return size;
}

/**
 * JSONを解析
 * @param json - 対象のjson
 * @param key_value - JsonPair構造体のアドレス この構造体に解析した値が格納される
 * @param pairs_count - `key:value`のペアを最大いくつ生成するか
 * @return 解析したペアの数
 */
int parseJson(char *json, KeyValue *key_value, int pairs_count)
{
    // NOTE: 引数jsonに渡された文字列を `{` , `}` , `:` , ` ` のいずれかで分割
    char *token = strtok(json, "{},: ");
    char *key, *value;
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
        copyStringSafely(key_value[i].key, key, sizeof(key_value[i].key));

        if (value != NULL)
        {
            // NOTE: `value`の値をpairsのvalueに格納
            copyStringSafely(key_value[i].value, value, sizeof(key_value[i].value));
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
    // NOTE: ファイルサイズを取得
    int file_size = getFileSize(file_path);
    if (isError(file_size))
    {
        // NOTE: ファイルサイズが0やファイルが存在しない場合は404を返す
        printf("Error get file size: %s\n", file_path);
        return HTTP_STATUS_NOT_FOUND;
    }

    // NOTE: ファイルを読み込み
    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        printf("Error opening file: %s, error: %s\n", file_path, strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }

    // NOTE: ファイルを読み込んで、bodyに格納
    fread(body, DATA_BLOCK_SIZE_FOR_READ, file_size, file);
    fclose(file);

    return HTTP_STATUS_OK;
}
