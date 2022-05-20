#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint8_t *data;
} Image;

Image image_load(void *data, size_t len);
void image_unload(Image *self);
