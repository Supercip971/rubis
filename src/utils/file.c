#include <utils/file.h>

Buffer read_file(const char *path)
{
    FILE *f = fopen(path, "r");

    if (f == NULL)
    {
        printf("(warn) can't open: %s\n", path);
        return (Buffer){};
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);

    fseek(f, 0, SEEK_SET);

    char *buffer = malloc(size);

    fread(buffer, size, 1, f);
    return (Buffer){
        .len = size,
        .buffer = buffer,
    };
}
