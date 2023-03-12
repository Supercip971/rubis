#include <config.h>
#include <obj/material.h>
#include <obj/scene.h>
#include <stdio.h>
#include "math/aabb.h"
#include "math/mat4.h"
#include "math/vec3.h"
#include "obj/img.h"
#include "obj/mesh.h"
#include "render/vulkan/vertex.h"
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

void scene_resize_textures(Scene *self)
{
    size_t maxw = 0;
    size_t maxh = 0;
    printf("texture count: %u \n", self->textures.length);
    for (int i = 0; i < self->textures.length; i++)
    {
        Image img = self->textures.data[i];
        if (img.width > maxw)
        {
            maxw = img.width;
        }
        if (img.height > maxh)
        {
            maxh = img.height;
        }
    }

    if (maxw > 2048)
    {
        maxw = 2048;
    }
    if (maxh > 2048)
    {
        maxh = 2048;
    }

    printf("resizing images for: %zux%zu\n", maxw, maxh);

    for (int i = 0; i < self->textures.length; i++)
    {

        Image old = self->textures.data[i];
        printf(" %ux%u -> %zux%zu \n", old.width, old.height, maxw, maxh);

        if (old.width == maxw && old.height == maxh)
        {
            continue;
        }
        Image new = image_resize(old, maxw, maxh);
        self->textures.data[i] = new;
        image_unload(&old);
    }
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

MeshCreation scene_start_mesh(Scene *self, Material material)
{
    (void)self;
    Mesh mesh = {
        .material_type = material.type,
        .material = material.data,
        .type = MESH_TRIANGLES,
        .aabb = {
            .min = vec3_create(0, 0, 0),
            .max = vec3_create(0, 0, 0),
        },
    };

    MeshTriangles d;
    vec_init(&d);
    return (MeshCreation){
        .mesh = mesh,
        .data = d,
    };
}

Triangle scene_mesh_triangle(Scene *self, int mesh_index, int triangle_index)
{
    Mesh *mesh = &self->meshes.data[mesh_index];
    Vec3 *data = &self->data.data[mesh->vertices.start + triangle_index * TRIANGLE_PACKED_COUNT];
    Triangle t = triangle_unpack(data);

    return t;
}

SVertex mesh_read_vertex(Scene *self, Mesh *from, int vertex)
{
    if (vertex >= from->vertices.end / SVERTEX_PACKED_COUNT)
    {
        printf("vertex out of bounds: %d >= %d\n", vertex, from->vertices.end / SVERTEX_PACKED_COUNT);
        abort();
    }
    Vec3 *data = &self->data.data[from->vertices.start + vertex * SVERTEX_PACKED_COUNT];
    return svertex_unpack(data);
}

void mesh_write_vertex(Scene *self, Mesh *from, int vertex, SVertex data)
{
    if (vertex >= from->vertices.end / SVERTEX_PACKED_COUNT)
    {
        printf("vertex out of bounds: %d >= %d\n", vertex, from->vertices.end / SVERTEX_PACKED_COUNT);
        abort();
    }
    Vec3 *dat = &self->data.data[from->vertices.start + vertex * SVERTEX_PACKED_COUNT];
    svertex_pack(dat, data);
}

void mesh_push_triangle(MeshCreation *mesh, Triangle triangle)
{

    if (mesh->data.length == 0)
    {
        mesh->mesh.aabb = aabb_create_triangle(triangle.a.pos, triangle.b.pos, triangle.c.pos);
    }
    else
    {
        // TODO: this is so dumb, we should add a AABB library
        AABB next = aabb_create_triangle(triangle.a.pos, triangle.b.pos, triangle.c.pos);

        mesh->mesh.aabb.min = vec3_min(mesh->mesh.aabb.min, next.min);
        mesh->mesh.aabb.max = vec3_max(mesh->mesh.aabb.max, next.max);
    }

    vec_push(&mesh->data, triangle);
}

void scene_end_mesh(Scene *self, MeshCreation *mesh)
{
    MeshTriangles *d = &mesh->data;

    for (int i = 0; i < d->length; i++)
    {
        Triangle triangle = d->data[i];

        Vec3 raw[TRIANGLE_PACKED_COUNT];

        triangle_pack(raw, triangle);

        for (int c = 0; c < TRIANGLE_PACKED_COUNT; c++)
        {

            scene_data_reference_push(self, &mesh->mesh.vertices, raw[c]);
        }
    }

    vec_push(&self->meshes, mesh->mesh);
    vec_deinit(&mesh->data);
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

void scene_ref_push_material_texture(Scene *self, Material *mat, PbrtMaterialImage image)
{
    scene_data_reference_push(self, &mat->data, (Vec3){image.id, image.tid, 0, 0});
    scene_data_reference_push(self, &mat->data, image.factor);
    scene_data_reference_push(self, &mat->data, (Vec3){image.offx, image.offy, image.scalex, image.scaley});
}

PbrtMaterialImage scene_get_pbrt(Scene *self, int offset)
{
    PbrtMaterialImage image = {
        .id = self->data.data[offset].x,
        .tid = self->data.data[offset].y,
        .factor = self->data.data[offset + 1],
        .offx = self->data.data[offset + 2].x,
        .offy = self->data.data[offset + 2].y,
        .scalex = self->data.data[offset + 2].z,
        .scaley = self->data.data[offset + 2]._padding,
    };

    return image;
}

Pbrt scene_get_full_pbrt(Scene *self, int offset)
{
    PbrtMaterialImage base = scene_get_pbrt(self, offset);
    PbrtMaterialImage normal = scene_get_pbrt(self, offset + 3);
    PbrtMaterialImage metallic_roughness = scene_get_pbrt(self, offset + 6);
    PbrtMaterialImage emit = scene_get_pbrt(self, offset + 9);

    return (Pbrt){
        .base = base,
        .normal = normal,
        .metallic_roughness = metallic_roughness,
        .emit = emit,
        .alpha = base.factor._padding,
        .is_color = (base.id == -1),
    };
}
Material scene_push_pbrt(Scene *self, Pbrt pbrt)
{
    Material mat = {
        .type = MATERIAL_PBRT,
        .data = {}};

    if (pbrt.is_color)
    {
        pbrt.base.id = -1;
    }

    pbrt.base.factor._padding = pbrt.alpha;
    scene_ref_push_material_texture(self, &mat, pbrt.base);
    scene_ref_push_material_texture(self, &mat, pbrt.normal);
    scene_ref_push_material_texture(self, &mat, pbrt.metallic_roughness);
    scene_ref_push_material_texture(self, &mat, pbrt.emit);

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
void scene_emissive_indices_init(Scene *self)
{
    vec_init(&self->mesh_emissive_indices);

    for (int i = 0; i < self->meshes.length; i++)
    {
        Mesh *mesh = &self->meshes.data[i];

        AABB m_aabb = mesh->aabb;

        float size = vec3_squared_length(vec3_sub(m_aabb.max, m_aabb.min));

        PbrtMaterialImage emit = scene_get_pbrt(self, mesh->material.start + 9);

        printf("[%i] %i | %f %f %f\n", i, emit.id, emit.factor.x, emit.factor.y, emit.factor.z);

        if (emit.factor.x + emit.factor.y + emit.factor.z < 3 && emit.id < 0)
        {
            continue;
        }
        if (emit.id == -1 && emit.factor.x + emit.factor.y + emit.factor.z < 20)
        {
            continue;
        }
        printf("founded emissive: %i | %f \n", i, size);

        for(int u = 0; u < (int)(fmax(1, size/10)); u++)
        {
            vec_push(&self->mesh_emissive_indices, (EmissiveIndex){i});
        }
   }
    printf("emissive count %i\n", self->mesh_emissive_indices.length);
}
