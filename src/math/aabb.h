#pragma once 
#include <math/vec3.h>

typedef struct __attribute__((packed))
{
    _Alignas(16) Vec3 min;
    _Alignas(16) Vec3 max;
} AABB;

static inline bool aabb_intersect(const AABB *a, const AABB *b)
{
    return (a->min.x <= b->max.x && a->max.x >= b->min.x) &&
           (a->min.y <= b->max.y && a->max.y >= b->min.y) &&
           (a->min.z <= b->max.z && a->max.z >= b->min.z);
}

static inline AABB aabb_inter(const AABB *a, const AABB *b)
{
    if(!aabb_intersect(a, b))
    {
        return (AABB){
            .min = vec3_create(0, 0, 0),
            .max = vec3_create(0, 0, 0),
        };
    }

    return (AABB){
        .min = vec3_max(a->min, b->min),
        .max = vec3_min(a->max, b->max),
    };
}

static inline Vec3 aabb_centroid(const AABB *a)
{
    Vec3 a_center = vec3_add(a->min, vec3_mul_val(vec3_sub(a->max, a->min), 0.5));

    return a_center;
}

static inline Vec3 aabb_offset(const AABB *a, Vec3 pos) 
{
    Vec3 a_size = vec3_sub(a->max, a->min);
    Vec3 a_offset = vec3_sub(pos, a->min);
    Vec3 a_offset_norm = vec3_div(a_offset, a_size);
    return a_offset_norm;
}
static inline AABB aabb_create(Vec3 min, Vec3 max)
{
    return (AABB){
        .min = vec3_min(min, max),
        .max = vec3_max(min, max),
    };
}
static inline AABB aabb_surrounding(const AABB *__restrict a, const AABB *__restrict b)
{

    return aabb_create(vec3_min(a->min, b->min), vec3_max(a->max, b->max));
}


static inline AABB aabb_create_triangle(Vec3 pa, Vec3 pb, Vec3 pc)
{
    AABB c =  (AABB){
        .min = vec3_min(pa, vec3_min(pb, pc)),
        .max = vec3_max(pa, vec3_max(pb, pc)),
    };

    if(c.max.x - c.min.x <= 0.00001)
    {
        c.min.x -= 0.00001;
        c.max.x += 0.00001;
    }
    if(c.max.y - c.min.y <= 0.00001)
    {
        c.min.y -= 0.00001;
        c.max.y += 0.00001;
    }
    if(c.max.z - c.min.z <= 0.00001)
    {
        c.min.z -= 0.00001;
        c.max.z += 0.00001;
    }
    return c;
}

static inline float aabb_centroid_distance(AABB l, AABB r)
{
    Vec3 l_center = aabb_centroid(&l);
    Vec3 r_center = aabb_centroid(&r);

    return vec3_length(vec3_sub(l_center, r_center));
}


static inline float aabb_near_same(AABB l, AABB r, float bias)
{
    Vec3 l_center = aabb_centroid(&l);
    Vec3 r_center = aabb_centroid(&r);

    return vec3_length(vec3_sub(l_center, r_center)) < bias;
}

static inline float aabb_surface_area(AABB l)
{
    Vec3 l_size =  vec3_sub(l.max, l.min);
  

    return (2 * (l_size.x * l_size.y + l_size.x * l_size.z + l_size.y * l_size.z));
}
