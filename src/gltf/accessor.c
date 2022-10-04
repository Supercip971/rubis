#include "accessor.h"
#include <string.h>

const char *type_str[] = {
    [GLTF_NONE] = "",
    [GLTF_SCALAR] = "SCALAR",
    [GLTF_VEC2] = "VEC2",
    [GLTF_VEC3] = "VEC3",
    [GLTF_VEC4] = "VEC4",
    [GLTF_MAT2] = "MAT2",
    [GLTF_MAT3] = "MAT3",
    [GLTF_MAT4] = "MAT4",
};

static GltfAcessorTypes get_accessor_type_from_str(const char *data)
{
    for (size_t i = 0; i < sizeof(type_str) / sizeof(type_str[0]); i++)
    {
        if (strcmp(data, type_str[i]))
        {
            return i;
        }
    }

    return GLTF_NONE;
}

void gltf_get_buffer_from_view(GltfCtx *ctx, int view_id, GltfBufView *res, int len, int off)
{
    cJSON *view_array = (cJSON_GetObjectItem(ctx->root, "bufferViews"));

    cJSON *view = cJSON_GetArrayItem(view_array, view_id);

    if (len == 0)
    {

        res->len = cJSON_GetObjectItem(view, "byteLength")->valueint;
    }
    else
    {
        res->len = len;
    }

    res->data = (uint8_t *)ctx->binary->data + cJSON_GetObjectItem(view, "byteOffset")->valueint + off;
}

GltfAccessorPtr gltf_read_accessor(GltfCtx *self, int idx)
{
    cJSON *accessor_array = (cJSON_GetObjectItem(self->root, "accessors"));
    cJSON *accessor = cJSON_GetArrayItem(accessor_array, idx);

    cJSON *off = cJSON_GetObjectItem(accessor, "byteOffset");
    int offset = 0;
    if (off)
    {
        offset = off->valueint;
    }
    GltfAccessorPtr ptr = {
        .componen_type = cJSON_GetObjectItem(accessor, "componentType")->valueint,
        .type = get_accessor_type_from_str(cJSON_GetObjectItem(accessor, "type")->valuestring),
        .count = cJSON_GetObjectItem(accessor, "count")->valueint,
        .off = offset,
    };

    int buffer_view_id = cJSON_GetObjectItem(accessor, "bufferView")->valueint;
    gltf_get_buffer_from_view(self, buffer_view_id, &ptr.view, 0, ptr.off);
    return ptr;
}
