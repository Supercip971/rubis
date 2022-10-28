#pragma once
#include <ds/vec.h>
#include <math/vec3.h>

#define MESH_NONE 0
#define MESH_CIRCLE 1
#define MESH_TRIANGLES 2

#define MESH_VERTICE_COUNT 11

typedef struct __attribute__((packed))
{
    _Alignas(4) int start;
    _Alignas(4) int end;
} DataReference;

typedef struct __attribute__((packed))
{
    _Alignas(16) Vec3 min;
    _Alignas(16) Vec3 max;
} AABB;
typedef struct __attribute__((packed))
{
    _Alignas(4) int type;
    _Alignas(4) int material_type;
    _Alignas(8) DataReference material; // for later
    _Alignas(8) DataReference vertices; // a list of vec4, a circle can use one for position, one for scale
    _Alignas(16) AABB aabb;
} Mesh;

typedef struct
{
    float x, y;
} TriangleTexPos;

typedef struct
{
    Vec3 pa;

    Vec3 pb;
    Vec3 pc;

    union
    {

        float tex_coords[3][2];
        TriangleTexPos tex_pos[3];
    };
    float _pad;

    Vec3 na;
    Vec3 nb;
    Vec3 nc;

    // tangent 
    Vec3 ta;
    Vec3 tb;
    Vec3 tc;



} Triangle;
typedef struct __attribute__((packed))
{

} MeshVertice;
typedef vec_t(Vec3) Points;
typedef vec_t(Mesh) Meshes;

void mesh_init(Meshes *self);

void mesh_add_circle(Meshes *self);

int mesh_count_faces(Mesh *self);

Triangle triangle_unpack(const Vec3* packed_data);
void triangle_pack(Vec3* target, Triangle triangle);

