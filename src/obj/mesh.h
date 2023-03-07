#pragma once
#include <ds/vec.h>
#include <math/vec3.h>
#include <math/aabb.h>
#include <render/vulkan/vertex.h>

#define MESH_NONE 0
#define MESH_CIRCLE 1
#define MESH_TRIANGLES 2

typedef struct __attribute__((packed))
{
    _Alignas(4) int start;
    _Alignas(4) int end;
} DataReference;

typedef struct __attribute__((packed))
{
    _Alignas(4) int type;
    _Alignas(4) int material_type;
    _Alignas(8) DataReference material; // for later
    _Alignas(8) DataReference vertices; // a list of vec4, a circle can use one for position, one for scale
    _Alignas(16) AABB aabb;
} Mesh;


typedef vec_t(Vec3) Points;
typedef vec_t(Mesh) Meshes;

void mesh_init(Meshes *self);

void mesh_add_circle(Meshes *self);

int mesh_count_faces(Mesh *self);
