#pragma once
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    size_t len;
    char *buffer;
} Buffer;

Buffer read_file(const char *path);
