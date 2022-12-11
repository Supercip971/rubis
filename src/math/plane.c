#include <math/plane.h>
#include "math/aabb.h"
#include "math/vec3.h"

bool is_point_inside_plane(AAPlane plane, Vec3 point)
{
    float da = vec3_dim(point, plane.dim) * -plane.sign + plane.t * plane.sign;
    return da >= 0;
}
bool is_aabb_inside_plane(AABB box, AAPlane plane)
{
    return is_point_inside_plane(plane, box.min) && is_point_inside_plane(plane, box.max);
}

static bool triangle_in_plane_aabb_single(AABB *result, Vec3 pa, Vec3 pb, Vec3 pc, AAPlane plane)
{
    // if only one point is positive, then there will be 1 triangle resulting from the split
    // the split happend where A is the positive point, and B and C are negative
    // if we have P the plane, and B' and C' the intersection of the line AB and AC with the plane
    // then the resulting triangle is AB'C'

    float da = vec3_dim(pa, plane.dim) * -plane.sign + plane.t * plane.sign;
    float db = vec3_dim(pb, plane.dim) * -plane.sign + plane.t * plane.sign;
    float dc = vec3_dim(pc, plane.dim) * -plane.sign + plane.t * plane.sign;

    if (!(da >= 0 && db <= 0 && dc <= 0))
    {
        return false;
    }

    Vec3 intersection_ab = plane_line_intersection(plane, pa, vec3_unit(vec3_sub(pb, pa)));
    Vec3 intersection_ac = plane_line_intersection(plane, pa, vec3_unit(vec3_sub(pc, pa)));

    *result = aabb_create(vec3_min(intersection_ab, vec3_min(pa, intersection_ac)), vec3_max(intersection_ab, vec3_max(pa, intersection_ac)));

    return true;
}

static bool triangle_in_plane_aabb_double(AABB *result, Vec3 pa, Vec3 pb, Vec3 pc, AAPlane plane)
{
    // if only one point is negative, then there will be 2 triangle resulting from the split
    // the split happend where A  & B are the positive points, and C is negative
    // if we have P the plane, and A' and B' the intersection of the line AC and BC with the plane
    // then the resulting triangles are: ABB' and BA'B'
    //

    float da = vec3_dim(pa, plane.dim) * -plane.sign + plane.t * plane.sign;
    float db = vec3_dim(pb, plane.dim) * -plane.sign + plane.t * plane.sign;
    float dc = vec3_dim(pc, plane.dim) * -plane.sign + plane.t * plane.sign;

    if (!(da >= 0 && db >= 0 && dc <= 0))
    {
        return false;
    }

    Vec3 intersection_ac = plane_line_intersection(plane, pa, vec3_unit(vec3_sub(pc, pa))); // A'
    Vec3 intersection_bc = plane_line_intersection(plane, pb, vec3_unit(vec3_sub(pc, pb))); // B'

    AABB b1 = aabb_create_triangle(pa, pb, intersection_bc);              // ABB'
    AABB b2 = aabb_create_triangle(pb, intersection_ac, intersection_bc); // BA'B'

    *result = aabb_surrounding(&b1, &b2);

    return true;
}

bool triangle_in_plane_aabb(Vec3 pa, Vec3 pb, Vec3 pc, AAPlane plane, AABB *result)
{
    // Ok, so we split with a bounding box a Triangle from a plane, and we want to know if the triangle is on the left or the right of the plane

    float da = vec3_dim(pa, plane.dim) * -plane.sign + plane.t * plane.sign;
    float db = vec3_dim(pb, plane.dim) * -plane.sign + plane.t * plane.sign;
    float dc = vec3_dim(pc, plane.dim) * -plane.sign + plane.t * plane.sign;

    // they are already all in the plane
    if (da >= 0.0f && db >= 0.0f && dc >= 0.0f)
    {
        *result = aabb_create_triangle(pa,pb,pc);        return true;
    }

    if (da < 0.0f && db < 0.0f && dc < 0.0f)
    {
        return false;
    }

    if (triangle_in_plane_aabb_single(result, pa, pb, pc, plane))
    {
        return true;
    }
    if (triangle_in_plane_aabb_single(result, pb, pa, pc, plane))
    {
        return true;
    }
    if (triangle_in_plane_aabb_single(result, pc, pb, pa, plane))
    {
        return true;
    }

    Vec3 pos_array[3] = {pa, pb, pc};
    // We must try:
    // A B C
    // A C B
    // B A C
    // B C A
    // C A B
    // C B A
    for (int i = 0; i < 3; i++)
    {
        Vec3 ca = pos_array[i];
        Vec3 cb = pos_array[0];
        Vec3 cc = pos_array[1];

        if (i == 0)
        {
            cb = pos_array[1];
            cc = pos_array[2];
        }
        else if (i == 1)
        {
            cb = pos_array[2];
            cc = pos_array[0];
        }
        else if (i == 2)
        {
            cb = pos_array[0];
            cc = pos_array[1];
        }
        if (triangle_in_plane_aabb_double(result, ca, cb, cc, plane))
        {
            return true;
        }
        if (triangle_in_plane_aabb_double(result, ca, cc, cb, plane))
        {
            return true;
        }
    }

    return false;
}

Vec3 plane_line_intersection(AAPlane plane, Vec3 p, Vec3 direction)
{
    float t = ((plane.t) - vec3_dim(p, plane.dim)) / vec3_dim(direction, plane.dim);
    return vec3_add(p, vec3_mul_val(direction, t));
}
