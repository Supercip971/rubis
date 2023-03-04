#include "math/vec3.h"
#include <stdio.h>
#include "obj/scene.h"
Triangle *mesh_get_triangle(MeshCreation *mesh, int index)
{
    return &mesh->data.data[index];
}
Vec3 mesh_geometry_normal(Triangle *input)
{
    Vec3 u = vec3_sub(input->pb, input->pa);
    Vec3 v = vec3_sub(input->pc, input->pa);

    return vec3_unit(vec3_cross(u, v));
}

int triangle_shared_point_count(Triangle *a, Triangle *b)
{
    int count = 0;
    if (vec3_eq(a->pa, b->pa) || vec3_eq(a->pa, b->pb) || vec3_eq(a->pa, b->pc))
    {
        count++;
    }
    if (vec3_eq(a->pb, b->pa) || vec3_eq(a->pb, b->pb) || vec3_eq(a->pb, b->pc))
    {
        count++;
    }
    if (vec3_eq(a->pc, b->pa) || vec3_eq(a->pc, b->pb) || vec3_eq(a->pc, b->pc))
    {
        count++;
    }
    return count;
}

void mesh_gen_normal_for_triangle(MeshCreation *mesh, int triangle_index)
{
    Triangle *triangle = mesh_get_triangle(mesh, triangle_index);

    Vec3 geometry_normal = mesh_geometry_normal(triangle);
    Vec3 result_a = geometry_normal;
    Vec3 result_b = geometry_normal;
    Vec3 result_c = geometry_normal;


    for (int i = 0; i < mesh->data.length; i++)
    {
        Triangle *other = mesh_get_triangle(mesh, i);

        Triangle *a = triangle;
        Triangle *b = other;
        if (triangle_shared_point_count(a, b) < 1)
        {
            continue;
        }
        Vec3 normal = mesh_geometry_normal(other);

        float angle = fabs(vec3_dot(geometry_normal, normal));
        if (vec3_eq(a->pa, b->pa) || vec3_eq(a->pa, b->pb) || vec3_eq(a->pa, b->pc))
        {
            result_a = vec3_add(vec3_mul_val(normal, angle), result_a);
        }
        if (vec3_eq(a->pb, b->pa) || vec3_eq(a->pb, b->pb) || vec3_eq(a->pb, b->pc))
        {
            result_b = vec3_add(vec3_mul_val(normal, angle), result_b);
        }
        if (vec3_eq(a->pc, b->pa) || vec3_eq(a->pc, b->pb) || vec3_eq(a->pc, b->pc))
        {
            result_c = vec3_add(vec3_mul_val(normal, angle), result_c);
        }
    }

    triangle->na = vec3_unit(result_a);
    triangle->nb = vec3_unit(result_b);
    triangle->nc = vec3_unit(result_c);

    triangle->has_normals = true;
}
void mesh_gen_normals_if_needed(MeshCreation *mesh)
{
    printf("generating normal for: %lx \n", (uintptr_t)mesh);
    for (int i = 0; i < mesh->data.length; i++)
    {
        Triangle *triangle = mesh_get_triangle(mesh, i);

        if (!triangle->has_normals)
        {
            mesh_gen_normal_for_triangle(mesh, i);
            triangle->has_normals = true;
        }
    }
}
