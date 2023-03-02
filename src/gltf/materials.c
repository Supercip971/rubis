
#include <cjson/cJSON.h>
#include <gltf/accessor.h>
#include <gltf/materials.h>
#include <stdio.h>
#include "gltf/gltf.h"
#include "math/vec3.h"
#include "obj/scene.h"

int read_texcoord(cJSON *v)
{
    cJSON *v2 = cJSON_GetObjectItem(v, "texCoord");
    if (v2 == NULL)
    {
        return 0;
    }
    return v->valueint;
}

int read_texture(PbrtMaterialImage *img, cJSON *v, GltfCtx *self)
{
    cJSON *v2 = cJSON_GetObjectItem(v, "index");
    if (v2 == NULL)
    {
        img->id = -1;
        return 0;
    }


    img->id = self->textures.data[v2->valueint];

    cJSON* tcoord = cJSON_GetObjectItem(v, "texCoord");
    if(tcoord)
    {
        img->tid = tcoord->valueint;
    }
    else
    {
        img->tid = 0;
    }


    img->offx = 0;
    img->offy = 0;

    img->scalex = 1;
    img->scaley = 1;

    cJSON *exts = cJSON_GetObjectItem(v, "extensions");
    if (!exts)
    {
        return 0;
    }

    cJSON *trans = cJSON_GetObjectItem(exts, "KHR_texture_transform");
    if (!trans)
    {
        return 0;
    }

    cJSON *scale = cJSON_GetObjectItem(trans, "scale");

    if (scale)
    {
        
        img->scalex = cJSON_GetArrayItem(scale, 0)->valuedouble;
        img->scaley = cJSON_GetArrayItem(scale, 1)->valuedouble;
        printf("img scale %f %f \n", img->scalex, img->scaley);

    }

    cJSON *off = cJSON_GetObjectItem(trans, "offset");

    if (off)
    {
        img->offx = cJSON_GetArrayItem(off, 0)->valuedouble;
        img->offy = cJSON_GetArrayItem(off, 1)->valuedouble;
        printf("img offset %f %f \n", img->offx, img->offy);


    }

    return 1;
}

void gltf_materials_parse(GltfCtx *self)
{
    vec_init(&self->materials);
    cJSON *materials_array = (cJSON_GetObjectItem(self->root, "materials"));
    int materials_count = cJSON_GetArraySize(materials_array);
    PbrtMaterialImage default_img = {
        .id = -1,
        .tid = 0,
        .factor = vec3$(1, 1, 1),
        .offx = 0,
        .offy = 0,
        .scalex = 1,
        .scaley = 1,
    };

    for (int i = 0; i < materials_count; i++)
    {
        printf("parsing material %i\n", i);
        cJSON *material = cJSON_GetArrayItem(materials_array, i);

        GltfMaterial current = {
            .base = default_img,
        };

        cJSON *pbr = cJSON_GetObjectItem(material, "pbrMetallicRoughness");

        current.metallic_roughness = default_img;

        current.normal = default_img;

        current.metallic_roughness = default_img;

        current.normal = default_img;

        current.alpha = 1.0;
        current.emit = default_img;
        current.normal.factor = vec3$(1, 1, 1);
        if (pbr != 0)
        {
            printf("has pbr\n");

            cJSON *base = cJSON_GetObjectItem(pbr, "baseColorTexture");
            cJSON *base_col = cJSON_GetObjectItem(pbr, "baseColorFactor");

            if (base != NULL)
            {

                read_texture(&current.base, base, self);

                printf("has base: %i\n", current.base.id);
            }
            else {
                current.is_color = true;
            }
            if (base_col != NULL)
            {
                float r = cJSON_GetArrayItem(base_col, 0)->valuedouble;

                float g = cJSON_GetArrayItem(base_col, 1)->valuedouble;
                float b = cJSON_GetArrayItem(base_col, 2)->valuedouble;
                float a = cJSON_GetArrayItem(base_col, 3)->valuedouble;

                printf("has color %f %f %f %f\n", r, g, b, a);
                current.alpha = a;
                current.base.factor = vec3$(r, g, b);
            }
            else  
            {
                current.base.factor = vec3$(1, 1, 1);
            }

            cJSON *metallic_roughness = cJSON_GetObjectItem(pbr, "metallicRoughnessTexture");
            if (metallic_roughness != NULL)
            {
                printf("has metallic roughness\n");
                read_texture(&current.metallic_roughness, metallic_roughness, self);

                //      int id = cJSON_GetObjectItem(metallic_roughness, "index")->valueint;
                //      current.metallic_roughness.id = self->textures.data[id];
                //      current.metallic_roughness.tid = read_texcoord(metallic_roughness);
            }
            cJSON *rougness_fact = cJSON_GetObjectItem(pbr, "roughnessFactor");
            if (rougness_fact != NULL)
            {
                current.metallic_roughness.factor.y = rougness_fact->valuedouble;

                printf("has metallic roughness factor %f \n", current.metallic_roughness.factor.y);
            }
            cJSON *metallic_fact = cJSON_GetObjectItem(pbr, "metallicFactor");
            if (metallic_fact != NULL)
            {
                current.metallic_roughness.factor.z = metallic_fact->valuedouble;

                printf("has metallic metal factor %f \n", current.metallic_roughness.factor.z);
            }
        }
        else
        {
            printf("no\n");
        }

        cJSON *normal = cJSON_GetObjectItem(material, "normalTexture");
        if (normal != NULL)
        {

            read_texture(&current.normal, normal, self);

            //            current.normal.id = self->textures.data[id];
            //            current.normal.tid = read_texcoord(normal);
            cJSON *normal_scale = cJSON_GetObjectItem(normal, "scale");
            if (normal_scale)
            {
                current.normal.factor = vec3$(normal_scale->valuedouble, normal_scale->valuedouble, 1.0);
            }
        }

        cJSON *emit = cJSON_GetObjectItem(material, "emissiveTexture");
        if (emit != NULL)
        {
            read_texture(&current.emit, emit, self);
        }

        else
        {
            current.emit.id = -1;
            current.emit.factor = vec3$(0, 0, 0);
        }

        cJSON *emit_fact = cJSON_GetObjectItem(material, "emissiveFactor");
        if (emit_fact != NULL)
        {
            current.emit.factor = vec3$(cJSON_GetArrayItem(emit_fact, 0)->valuedouble, cJSON_GetArrayItem(emit_fact, 1)->valuedouble, cJSON_GetArrayItem(emit_fact, 2)->valuedouble);

            cJSON *emissive_stren = cJSON_GetObjectItem(
                cJSON_GetObjectItem(
                    cJSON_GetObjectItem(material, "extensions"),
                    "KHR_materials_emissive_strength"),
                "emissiveStrength");

            if (emissive_stren != NULL)
            {
                current.emit.factor = vec3_mul_val(current.emit.factor, emissive_stren->valuedouble);
            }
        }

        Pbrt final = (Pbrt){
            .base = current.base,
            .is_color = current.is_color,
            .normal = current.normal,
            .emit = current.emit,
            .metallic_roughness = current.metallic_roughness,
            .alpha = current.alpha,
        };
        current.final = scene_push_pbrt(self->target, final);

        vec_push(&self->materials, current);
    }

    self->null_material_id = self->materials.length;

    Pbrt final = {
        .base = {
            .id = -1,
            .factor = vec3$(1, 1, 1),
        },
        .is_color = true,
        .normal = {
            .id = -1,
            .factor = vec3$(1, 1, 1),
        },
        .emit = {
            .id = -1,
            .factor = vec3$(0, 0, 0),
        },
        .metallic_roughness = {
            .id = -1,
            .factor = vec3$(0, 0, 0.5),
        },
        .alpha = 1.0,
    };

    GltfMaterial material = {
        .base = default_img};
    material.final = scene_push_pbrt(self->target, final);

    vec_push(&self->materials, material);
}
