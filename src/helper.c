#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "helper.h"
#include "constance.h"

void copyStringSafely(char *destination, char *source, size_t destination_size)
{
    strncpy(destination, source, destination_size - 1);
    destination[destination_size - 1] = '\0';
}

int isError(int target)
{
    if (target == ERROR_FLAG)
    {
        return 1;
    }
    return 0;
}