#include <obj/material.h>
#include <obj/scene.h>
#include "math/aabb.h"
#include "math/mat4.h"
#include "math/vec3.h"
#include "obj/img.h"
#include <config.h>
#include "obj/mesh.h"
#include <stdio.h>
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

void scene_resize_textures(Scene* self)
{
    size_t maxw = 0;
    size_t maxh = 0;
    printf("texture count: %u \n", self->textures.length);
    for(int i = 0; i < self->textures.length; i++)
    {
        Image img = self->textures.data[i];
        if(img.width > maxw)
        {
            maxw = img.width;
        }
        if(img.height > maxh)
        {
            maxh = img.height;
        }
    }


    printf("resizing images for: %zux%zu\n", maxw, maxh);

    
    for(int i = 0; i < self->textures.length; i++)
    {

        Image old = self->textures.data[i];
        printf(" %ux%u -> %zux%zu \n", old.width, old.height, maxw, maxh);



        if(old.width == maxw && old.height == maxh)
        {
            continue;
        }
        Image new = image_resize(old, maxw,maxh);
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

void scene_push_tri(Scene *self, Vec3 posa, Vec3 posb, Vec3 posc, Material material)
{
    Mesh mesh = {
        .material_type = material.type,
        .material = material.data,
        .type = MESH_TRIANGLES,
        .aabb = 
            aabb_create_triangle(posa, posb, posc)
        ,
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
        .aabb =             aabb_create_triangle(triangle.pa, triangle.pb, triangle.pc),

    };

    scene_data_reference_push(self, &mesh.vertices, triangle.pa);
    scene_data_reference_push(self, &mesh.vertices, triangle.pb);

    scene_data_reference_push(self, &mesh.vertices, triangle.pc);

    Vec3 tcoord1;
    Vec3 tcoord2;

    // TEXCOORD_0
    tcoord1.x = triangle.tc1.tex_coords[0][0]; // pa.x
    tcoord1.y = triangle.tc1.tex_coords[0][1]; // pa.y
    tcoord1.z = triangle.tc1.tex_coords[1][0]; // pb.x
    tcoord2.x = triangle.tc1.tex_coords[1][1]; // pb.y
    tcoord2.y = triangle.tc1.tex_coords[2][0]; // pc.x
    tcoord2.z = triangle.tc1.tex_coords[2][1]; // pc.y

    scene_data_reference_push(self, &mesh.vertices, tcoord1);
    scene_data_reference_push(self, &mesh.vertices, tcoord2);

    // TEXCOORD_1
    tcoord1.x = triangle.tc2.tex_coords[0][0]; // pa.x
    tcoord1.y = triangle.tc2.tex_coords[0][1]; // pa.y
    tcoord1.z = triangle.tc2.tex_coords[1][0]; // pb.x
    tcoord2.x = triangle.tc2.tex_coords[1][1]; // pb.y
    tcoord2.y = triangle.tc2.tex_coords[2][0]; // pc.x
    tcoord2.z = triangle.tc2.tex_coords[2][1]; // pc.y

    scene_data_reference_push(self, &mesh.vertices, tcoord1);
    scene_data_reference_push(self, &mesh.vertices, tcoord2);


    scene_data_reference_push(self, &mesh.vertices, triangle.na);

    scene_data_reference_push(self, &mesh.vertices, triangle.nb);
    scene_data_reference_push(self, &mesh.vertices, triangle.nc);

    // Tangent
    scene_data_reference_push(self, &mesh.vertices, triangle.ta);
    scene_data_reference_push(self, &mesh.vertices, triangle.tb);
    scene_data_reference_push(self, &mesh.vertices, triangle.tc);

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
    Vec3 *data = &self->data.data[mesh->vertices.start + triangle_index * MESH_VERTICE_COUNT];
    Triangle t  = triangle_unpack(data);

    return t;
}
void mesh_push_triangle( MeshCreation *mesh, Triangle triangle)
{

    if(mesh->data.length == 0)
    {
        mesh->mesh.aabb = aabb_create_triangle(triangle.pa, triangle.pb, triangle.pc);
    }
    else
    {
        // TODO: this is so dumb, we should add a AABB library
        AABB next = aabb_create_triangle(triangle.pa, triangle.pb, triangle.pc);
    
        mesh->mesh.aabb.min = vec3_min(mesh->mesh.aabb.min, next.min);
        mesh->mesh.aabb.max = vec3_max(mesh->mesh.aabb.max, next.max);
    }

    vec_push(&mesh->data, triangle);
}

void scene_end_mesh(Scene *self, MeshCreation* mesh)
{
    MeshTriangles *d = &mesh->data;

    for (int i = 0; i < d->length; i++)
    {
        Triangle triangle = d->data[i];

        Vec3 raw[MESH_VERTICE_COUNT];

        triangle_pack(raw, triangle);

        for(int c = 0;c < MESH_VERTICE_COUNT; c++)
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


void scene_ref_push_material_texture(Scene* self, Material* mat, PbrtMaterialImage image)
{
    scene_data_reference_push(self, &mat->data, (Vec3){image.id, image.tid, 0, 0});
    scene_data_reference_push(self, &mat->data, image.factor);
    scene_data_reference_push(self, &mat->data, (Vec3){image.offx, image.offy, image.scalex, image.scaley});
}

PbrtMaterialImage scene_get_pbrt(Scene* self, int offset)
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
