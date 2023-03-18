#pragma once
#include <ds/vec.h>
#include <math/vec3.h>
#include <obj/scene.h>

typedef struct __attribute__((packed))
{
    _Alignas(4) int is_next_a_bvh;
    _Alignas(16) AABB box;

    // LA is the index of the mesh, and LB is the element of the mesh (triangle) 
    _Alignas(4) int la;
    _Alignas(4) int lb; 
    _Alignas(4) int ra;
    _Alignas(4) int rb;
} BvhEntry;

typedef vec_t(BvhEntry) BvhList;

typedef struct
{
    BvhEntry entry;
    void *next_l;
    void *next_r;

} ElementOnList;

typedef vec_t(ElementOnList) tempBvhList;

tempBvhList sort_on_axis(const tempBvhList *list, VecDimension axis, bool sign);


void bvh_init(BvhList *self, Scene *target);
