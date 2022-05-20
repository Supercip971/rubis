#pragma once

#include <cjson/cJSON.h>
#include <cjson/cJSON_Utils.h>
#include <gltf/gltf.h>

typedef enum
{
    GLTF_COMP_NONE,
    GLTF_COMP_S8 = 5120,
    GLTF_COMP_U8 = 5121,
    GLTF_COMP_S16 = 5122,
    GLTF_COMP_U16 = 5123,
    GLTF_COMP_U32 = 5125,
    GLTF_COMP_FLOAT = 5126,
} GltfAcessorComponentTypes;

typedef enum
{
    GLTF_NONE,
    GLTF_SCALAR,

    GLTF_VEC2,
    GLTF_VEC3,
    GLTF_VEC4,
    GLTF_MAT2,
    GLTF_MAT3,
    GLTF_MAT4
} GltfAcessorTypes;

typedef struct
{
    void *data;
    size_t len;
} GltfBufView;

typedef struct
{
    GltfBufView view;
    int count;
    int off;
    GltfAcessorTypes type;
    GltfAcessorComponentTypes componen_type;
} GltfAccessorPtr;

// if len == 0 auto select
void gltf_get_buffer_from_view(GltfCtx *ctx, int view_id, GltfBufView *res, int len, int off);

GltfAccessorPtr gltf_read_accessor(GltfCtx *self, int idx);
