#include <obj/img.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>
Image image_load(void *data, size_t len)
{
    volatile int width = 0;
    volatile int height = 0;
    int channel_count = 0;

    void *d = stbi_load_from_memory(data, len, (int *)&width, (int *)&height, &channel_count, 4);

    return (Image){
        .data = d,
        .height = height,
        .width = width,
    };
}

Image image_resize(Image from, size_t new_width, size_t new_height)
{
    void* data = malloc(sizeof(uint32_t)*new_height*new_width);

    stbir_resize_uint8(from.data, from.width, from.height, 0, data, new_width, new_height,0, 4);

    return (Image){
        .data = data, 
        .width = new_width, 
        .height = new_height,
    };
}
void image_unload(Image *self)
{
    free(self->data);
}
