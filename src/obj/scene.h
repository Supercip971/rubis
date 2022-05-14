#pragma once

#include <obj/mesh.h>
#include <stddef.h>

typedef struct
{
    Points data;
    Meshes meshes;
} Scene;

typedef struct
{
    int type;
    DataReference data;
} Material;

void scene_init(Scene *self);

void scene_push_circle(Scene *self, Vec3 pos, float r, Material material);

Material scene_push_lambertian(Scene *self, Vec3 color);

Material scene_push_metal(Scene *self, Vec3 color, float fuzzy);

Material scene_push_dieletric(Scene *self, float r);

void scene_deinit(Scene *self);

void scene_build_buffer(Scene *self);

Material scene_push_light(Scene *self, Vec3 color);
