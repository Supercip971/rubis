#include <obj/material.h>
#include <obj/scene.h>
#include "math/mat4.h"

void scene_init(Scene *self)
{
    vec_init(&self->data);
    vec_init(&self->meshes);
    vec_init(&self->textures);
    create_matrix_identity(&self->camera_transform);
    // just add the entry n°0 so we can avoid to use this number
    vec_push(&self->data, (Vec3){});
}

imageID scene_push_texture(Scene *self, Image image)
{
    vec_push(&self->textures, image);
    return self->textures.length - 1;
}

void scene_data_reference_push(Scene *self, DataReference *dat, Vec3 value)
{
    if (dat->start == 0)
    {
        dat->start = self->data.length;
    }

    vec_push(&self->data, value);

    dat->end = self->data.length;
}

void scene_push_circle(Scene *self, Vec3 pos, float r, Material material)
{
    Mesh mesh = {
        .material_type = material.type,
        .material = material.data,
        .type = MESH_CIRCLE,
        .aabb = {
            .min = vec3_sub(pos, vec3$(r, r, r)),
            .max = vec3_add(pos, vec3$(r, r, r)),
        },
    };

    scene_data_reference_push(self, &mesh.vertices, pos);
    scene_data_reference_push(self, &mesh.vertices, vec3$(r, r, r));

    vec_push(&self->meshes, mesh);
}

void scene_push_tri(Scene *self, Vec3 posa, Vec3 posb, Vec3 posc, Material material)
{
    Mesh mesh = {
        .material_type = material.type,
        .material = material.data,
        .type = MESH_TRIANGLES,
        .aabb = {
            .min = vec3_min(posa, vec3_min(posb, posc)),
            .max = vec3_max(posa, vec3_max(posb, posc)),
        },
    };

    scene_data_reference_push(self, &mesh.vertices, posa);
    scene_data_reference_push(self, &mesh.vertices, posb);

    scene_data_reference_push(self, &mesh.vertices, posc);

    vec_push(&self->meshes, mesh);
}
void scene_push_tri2(Scene *self, Triangle triangle, Material material)
{
    Mesh mesh = {
        .material_type = material.type,
        .material = material.data,
        .type = MESH_TRIANGLES,
        .aabb = {
            .min = vec3_min(triangle.pa, vec3_min(triangle.pb, triangle.pc)),
            .max = vec3_max(triangle.pa, vec3_max(triangle.pb, triangle.pc)),
        },
    };

    scene_data_reference_push(self, &mesh.vertices, triangle.pa);
    scene_data_reference_push(self, &mesh.vertices, triangle.pb);

    scene_data_reference_push(self, &mesh.vertices, triangle.pc);

    Vec3 tcoord1;
    Vec3 tcoord2;

    tcoord1.x = triangle.tex_coords[0][0]; // pa.x
    tcoord1.y = triangle.tex_coords[0][1]; // pa.y
    tcoord1.z = triangle.tex_coords[1][0]; // pb.x
    tcoord2.x = triangle.tex_coords[1][1]; // pb.y
    tcoord2.y = triangle.tex_coords[2][0]; // pc.x
    tcoord2.z = triangle.tex_coords[2][1]; // pc.y

    scene_data_reference_push(self, &mesh.vertices, tcoord1);
    scene_data_reference_push(self, &mesh.vertices, tcoord2);
    scene_data_reference_push(self, &mesh.vertices, triangle.na);

    scene_data_reference_push(self, &mesh.vertices, triangle.nb);
    scene_data_reference_push(self, &mesh.vertices, triangle.nc);

    vec_push(&self->meshes, mesh);
}
Material scene_push_lambertian(Scene *self, Vec3 color)
{
    Material mat = {
        .type = MATERIAL_LAMBERTIAN,
        .data = {}};

    scene_data_reference_push(self, &mat.data, color);
    return mat;
}
Material scene_push_light(Scene *self, Vec3 color)
{
    Material mat = {
        .type = MATERIAL_LIGHT,
        .data = {}};

    scene_data_reference_push(self, &mat.data, color);
    return mat;
}
Material scene_push_metal(Scene *self, Vec3 color, float fuzzy)
{
    Material mat = {
        .type = MATERIAL_METAL,
        .data = {}};

    scene_data_reference_push(self, &mat.data, color);

    scene_data_reference_push(self, &mat.data, vec3$(fuzzy, 0, 0));
    return mat;
}

Material scene_push_pbrt(Scene *self, Pbrt pbrt)
{
    Material mat = {
        .type = MATERIAL_PBRT,
        .data = {}};

    float bret = pbrt.base;
    if (pbrt.is_color)
    {
        bret = -1;
    }
    scene_data_reference_push(self, &mat.data, (Vec3){pbrt.normal, bret, pbrt.roughness, pbrt.emit});
    pbrt.color._padding = pbrt.alpha;
    scene_data_reference_push(self, &mat.data, pbrt.color);
    scene_data_reference_push(self, &mat.data, vec3$(pbrt.metallic_fact, pbrt.rougness_fact, pbrt.normal_mul));
    scene_data_reference_push(self, &mat.data, pbrt.emmisive_fact);

    return mat;
}
Material scene_push_dieletric(Scene *self, float r)
{
    Material mat = {
        .type = MATERIAL_DIELETRIC,
        .data = {}};

    scene_data_reference_push(self, &mat.data, vec3$(r, 0, 0));
    return mat;
}
void scene_deinit(Scene *self)
{
    vec_deinit(&self->data);
    vec_deinit(&self->meshes);
}
