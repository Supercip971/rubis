#include <obj/img.h>
#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>
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
void image_unload(Image *self)
{
    stbi_image_free(self->data);
}
