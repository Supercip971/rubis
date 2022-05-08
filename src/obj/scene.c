#include <obj/scene.h>
#include <obj/material.h>

void scene_init(Scene* self)
{
    vec_init(&self->data);
    vec_init(&self->meshes);
    // just add the entry n°0 so we can avoid to use this number
    vec_push(&self->data, (Vec3){});
    vec_push(&self->meshes, (Mesh){});
}


void scene_data_reference_push(Scene* self, DataReference* dat, Vec3 value)
{
    if(dat->start == 0)
    {
        dat->start = self->data.length;
    }

    vec_push(&self->data, value);

    dat->end = self->data.length;
}

void scene_push_circle(Scene* self, Vec3 pos, float r, Material  material)
{
    Mesh mesh = {
        .material_type = material.type,
        .material = material.data,
        .type = MESH_CIRCLE,
    };

    scene_data_reference_push(self, &mesh.vertices, pos);
    scene_data_reference_push(self, &mesh.vertices, vec3$(r,r,r));

    vec_push(&self->meshes, mesh);
}

Material scene_push_lambertian(Scene* self, Vec3 color)
{
    Material mat = {
        .type = MATERIAL_LAMBERTIAN,
        .data =  {}
    };

    scene_data_reference_push(self, &mat.data, color);
    return mat;
}
Material scene_push_metal(Scene* self, Vec3 color, float fuzzy)
{
    Material mat = {
        .type = MATERIAL_METAL,
        .data =  {}
    };

    scene_data_reference_push(self, &mat.data, color);

    scene_data_reference_push(self, &mat.data, vec3$(fuzzy,0,0));
    return mat;
}


Material scene_push_dieletric(Scene* self, float r)
{
    Material mat = {
        .type = MATERIAL_DIELETRIC,
        .data =  {}
    };

    scene_data_reference_push(self, &mat.data, vec3$(r,0,0));
    return mat;
}
void scene_deinit(Scene* self)
{
    vec_deinit(&self->data);
    vec_deinit(&self->meshes);
}
