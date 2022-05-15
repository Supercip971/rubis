#pragma once

// actually it's more like gltf binary parsing because i'm too dumb
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

bool parse_gltf(void *data, Scene *target);
