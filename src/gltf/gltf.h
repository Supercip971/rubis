#pragma once

// actually it's more like gltf binary parsing because i'm too dumb
#include <cjson/cJSON.h>
#include <cjson/cJSON_Utils.h>
#include <obj/scene.h>
#include <stddef.h>
#include <stdint.h>

#define GLTF_MAGIC 0x46546C67

#define GLTF_TYPE_JSON 0x4E4F534A
#define GLTF_TYPE_BINARY 0x004E4942

typedef struct
{
    uint32_t len;
    uint32_t type;
    uint8_t data[];
} GltfChunkHeader;

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t len;

} GltfHeader;

typedef struct
{
    imageID normal;
    imageID base;
    imageID metallic_roughness;
    Material final;
} GltfMaterial;

typedef vec_t(imageID) GltfTextures;

typedef vec_t(GltfMaterial) GltfMaterials;

typedef struct
{
    Scene *target;
    void *data;
    cJSON *root;
    GltfChunkHeader *binary;
    GltfTextures textures;
    GltfMaterials materials;
} GltfCtx;

bool parse_gltf(void *data, Scene *target);
