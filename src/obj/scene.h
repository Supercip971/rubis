#pragma once

#include <obj/img.h>
#include <obj/mesh.h>
#include <stddef.h>
#include "math/mat4.h"
#include "math/vec3.h"
#include "render/vulkan/vertex.h"

#define PBRT_SIZE_IMAGE 3
typedef vec_t(Image) TexLists;

typedef struct 
{
    _Alignas(16) unsigned int index;
} EmissiveIndex;
typedef struct
{
    Points data;
    Meshes meshes;
    TexLists textures;
    Image skymap;

    Matrix4x4 camera_transform;
    float camera_fov;
    vec_t(EmissiveIndex) mesh_emissive_indices;
} Scene;

typedef struct
{
    int type;
    DataReference data;
} Material;




Triangle scene_mesh_triangle(Scene *self, int mesh_index, int triangle_index);

SVertex mesh_read_vertex(Scene* self, Mesh* from,   int vertex);
void mesh_write_vertex(Scene* self, Mesh* from, int vertex, SVertex data);

void scene_init(Scene *self);

void scene_resize_textures(Scene* self);
void scene_emissive_indices_init(Scene* self);

void scene_push_circle(Scene *self, Vec3 pos, float r, Material material);

void scene_push_tri(Scene *self, Vec3 posa, Vec3 posb, Vec3 posc, Material material);

//void scene_push_tri2(Scene *self, Triangle triangle, Material material);

typedef vec_t(Triangle) MeshTriangles;
typedef struct 
{
    bool has_tangent;
    Mesh mesh; 
    MeshTriangles data;
} MeshCreation;


MeshCreation scene_start_mesh(Scene* self, Material material);

void mesh_push_triangle( MeshCreation* mesh, Triangle triangle);

void scene_end_mesh(Scene* self, MeshCreation*  mesh);

void mesh_gen_normals_if_needed(MeshCreation *mesh);

bool scene_generate_tangent(Scene* self);

imageID scene_push_texture(Scene *self, Image image);

Material scene_push_lambertian(Scene *self, Vec3 color);

Material scene_push_metal(Scene *self, Vec3 color, float fuzzy);

Material scene_push_dieletric(Scene *self, float r);


typedef struct 
{
    float scalex, scaley; 
    float offx, offy; 
    Vec3 factor;
    int id;  // if id == -1 use factor instead
    int tid;
} PbrtMaterialImage;

#define DEFAULT_PBRT_IMAGE {1, 1, 0, 0, -1, -1}
typedef struct
{
    bool is_color;
    PbrtMaterialImage normal;
    PbrtMaterialImage base; // only if is_color == false
    PbrtMaterialImage metallic_roughness;
    PbrtMaterialImage emit;
    float alpha;

} Pbrt;

Material scene_push_pbrt(Scene *self, Pbrt pbrt);

PbrtMaterialImage scene_get_pbrt(Scene* self, int offset);

Pbrt scene_get_full_pbrt(Scene *self, int offset);
void scene_deinit(Scene *self);

void scene_emissive_indices_init(Scene* self);

void scene_build_buffer(Scene *self);

Material scene_push_light(Scene *self, Vec3 color);
