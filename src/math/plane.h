#pragma once 
#include <math/vec3.h>
#include <math/aabb.h>

typedef struct 
{
    VecDimension dim;
    float t; 
    float sign;
} AAPlane;


bool is_point_inside_plane(AAPlane plane, Vec3 point);

bool triangle_in_plane_aabb(Vec3 pa, Vec3 pb, Vec3 pc, AAPlane plane, AABB* result);

bool is_aabb_inside_plane(AABB box, AAPlane plane);

Vec3 plane_line_intersection(AAPlane plane, Vec3 p, Vec3 direction);
