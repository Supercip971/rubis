#pragma once 


#include <stddef.h>

#include <obj/mesh.h>

typedef struct  {
    Points data;
    Meshes meshes;
} Scene;

typedef struct
{
    int type;
    DataReference data;
} Material;

void scene_init(Scene* self);

void scene_push_circle(Scene* self, Vec3 pos, float r, Material  material);

Material scene_push_lambertian(Scene* self, Vec3 color);

void scene_deinit(Scene* self);

void scene_build_buffer(Scene* self);
