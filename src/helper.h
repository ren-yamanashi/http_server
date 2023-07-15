#ifndef _HELPER_H
#define _HELPER_H

#include <stdio.h>

void copyStringSafely(char *destination, char *source, size_t destination_size);
int isError(int target);
int isMatchStr(const char *str1, const char *str2);

#endif