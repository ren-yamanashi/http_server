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