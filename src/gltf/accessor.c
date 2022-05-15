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

void gltf_get_buffer_from_view(cJSON *gltf, int view_id, GltfChunkHeader const *binary, GltfAccessorPtr *res)
{
    cJSON *view_array = (cJSON_GetObjectItem(gltf, "bufferViews"));

    cJSON *view = cJSON_GetArrayItem(view_array, view_id);

    res->len = cJSON_GetObjectItem(view, "byteLength")->valueint;
    res->data = (uint8_t *)binary->data + cJSON_GetObjectItem(view, "byteOffset")->valueint;
}

GltfAccessorPtr gltf_read_accessor(cJSON *gltf, int idx, GltfChunkHeader const *binary)
{

    cJSON *accessor_array = (cJSON_GetObjectItem(gltf, "accessors"));

    cJSON *accessor = cJSON_GetArrayItem(accessor_array, idx);

    GltfAccessorPtr ptr = {};
    ptr.componen_type = cJSON_GetObjectItem(accessor, "componentType")->valueint;
    ptr.type = get_accessor_type_from_str(cJSON_GetObjectItem(accessor, "type")->valuestring);
    ptr.count = cJSON_GetObjectItem(accessor, "count")->valueint;
    int buffer_view_id = cJSON_GetObjectItem(accessor, "bufferView")->valueint;
    gltf_get_buffer_from_view(gltf, buffer_view_id, binary, &ptr);
    return ptr;
}
