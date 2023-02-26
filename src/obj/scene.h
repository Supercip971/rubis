#pragma once

#include <obj/img.h>
#include <obj/mesh.h>
#include <stddef.h>
#include "math/mat4.h"

typedef vec_t(Image) TexLists;
typedef struct
{
    Points data;
    Meshes meshes;
    Meshes reordred;
    TexLists textures;
    Image skymap;

    Matrix4x4 camera_transform;
    float camera_fov;
} Scene;

typedef struct
{
    int type;
    DataReference data;
} Material;




Triangle scene_mesh_triangle(Scene *self, int mesh_index, int triangle_index);

void scene_init(Scene *self);

void scene_resize_textures(Scene* self);
void scene_push_circle(Scene *self, Vec3 pos, float r, Material material);

void scene_push_tri(Scene *self, Vec3 posa, Vec3 posb, Vec3 posc, Material material);

//void scene_push_tri2(Scene *self, Triangle triangle, Material material);

typedef vec_t(Triangle) MeshTriangles;
typedef struct 
{
    Mesh mesh; 
    MeshTriangles data;
} MeshCreation;


MeshCreation scene_start_mesh(Scene* self, Material material);

void mesh_push_triangle( MeshCreation* mesh, Triangle triangle);

void scene_end_mesh(Scene* self, MeshCreation*  mesh);


bool scene_generate_tangent(Scene* self);

imageID scene_push_texture(Scene *self, Image image);

Material scene_push_lambertian(Scene *self, Vec3 color);

Material scene_push_metal(Scene *self, Vec3 color, float fuzzy);

Material scene_push_dieletric(Scene *self, float r);

typedef struct
{
    bool is_color;
    Vec3 color;
    int normal_tid;
    int base_tid;
    int roughness_tid;
    imageID normal;
    imageID base; // only if is_color == false
    imageID roughness;
    imageID emit;
    float rougness_fact; // if -1 none
    float alpha;
    float metallic_fact; // if -1 none
    float normal_mul;
    Vec3 emmisive_fact;
} Pbrt;

Material scene_push_pbrt(Scene *self, Pbrt pbrt);

void scene_deinit(Scene *self);

void scene_build_buffer(Scene *self);

Material scene_push_light(Scene *self, Vec3 color);
