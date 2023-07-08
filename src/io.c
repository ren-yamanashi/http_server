#include <sys/socket.h>
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
 * @param keyValue JsonPair構造体のアドレス この構造体に解析した値が格納される
 * @param pairsCount `key:value`のペアを最大いくつ生成するか
 * @return 解析したペアの数
 */
int parseJson(char *json, KeyValue *keyValue, int pairsCount)
{
    char *key, *value;
    // NOTE: 引数jsonに渡された文字列を `{` , `}` , `:` , ` ` のいずれかで分割
    char *token = strtok(json, "{},: ");
    int i = 0;

    while (token != NULL && i < pairsCount)
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
        strncpy(keyValue[i].key, key, sizeof(keyValue[i].key) - 1);
        keyValue[i].key[sizeof(keyValue[i].key) - 1] = '\0';

        if (value != NULL)
        {
            // NOTE: `value`の値をpairsのvalueに格納
            strncpy(keyValue[i].value, value, sizeof(keyValue[i].value) - 1);
            keyValue[i].value[sizeof(keyValue[i].value) - 1] = '\0';
        }
        else
        {
            keyValue[i].value[0] = '\0';
        }

        i++;
    }

    return i;
}