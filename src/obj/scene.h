#pragma once

#include <obj/img.h>
#include <obj/mesh.h>
#include <stddef.h>
typedef int imageID;

typedef vec_t(Image) TexLists;
typedef struct
{
    Points data;
    Meshes meshes;
    Meshes reordred;
    TexLists textures;

} Scene;

typedef struct
{
    int type;
    DataReference data;
} Material;

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

} Triangle;

void scene_init(Scene *self);

void scene_push_circle(Scene *self, Vec3 pos, float r, Material material);

void scene_push_tri(Scene *self, Vec3 posa, Vec3 posb, Vec3 posc, Material material);

void scene_push_tri2(Scene *self, Triangle triangle, Material material);

imageID scene_push_texture(Scene *self, Image image);

Material scene_push_lambertian(Scene *self, Vec3 color);

Material scene_push_metal(Scene *self, Vec3 color, float fuzzy);

Material scene_push_dieletric(Scene *self, float r);

Material scene_push_pbrt(Scene *self, imageID normal, imageID base, imageID metallic_roughness);

void scene_deinit(Scene *self);

void scene_build_buffer(Scene *self);

Material scene_push_light(Scene *self, Vec3 color);
