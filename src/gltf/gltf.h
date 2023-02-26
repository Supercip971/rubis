#pragma once

// actually it's more like gltf binary parsing because i'm too dumb
#include <cjson/cJSON.h>
#include <cjson/cJSON_Utils.h>
#include <math/mat4.h>
#include <obj/scene.h>
#include <stddef.h>
#include <stdint.h>
#include "obj/img.h"

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
    Vec3 scale;
    Vec3 translate;
    float rotate[4];
    Matrix4x4 raw;
} GltfTransforms;

typedef struct 
{
    imageID id; 
    int tex_coord;
} ImageMatRef;
    
typedef struct
{
    ImageMatRef normal;
    ImageMatRef base;
    ImageMatRef metallic_roughness;
    ImageMatRef emit;
    bool is_color;
    Vec3 color;
    float alpha;
    float rougness_fact; // if -1 none
    float metallic_fact; // if -1 none
    float normal_mul;
    
    Material final;
    Vec3 emissive_fact;
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
    int null_material_id;
} GltfCtx;

bool parse_gltf(void *data, Scene *target);
