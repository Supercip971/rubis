#pragma once
#include <ds/vec.h>
#include <math/vec3.h>
#include <obj/scene.h>

typedef struct __attribute__((packed))
{
    _Alignas(4) int is_next_a_bvh;
    _Alignas(16) AABB box;
    _Alignas(4) int l;
    _Alignas(4) int r;
} BvhEntry;

typedef vec_t(BvhEntry) BvhList;

void bvh_init(BvhList *self, Scene *target);
