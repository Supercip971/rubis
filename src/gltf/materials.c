
#include <cjson/cJSON.h>
#include <gltf/accessor.h>
#include <gltf/materials.h>
#include <stdio.h>
#include "gltf/gltf.h"
#include "math/vec3.h"

void gltf_materials_parse(GltfCtx *self)
{
    vec_init(&self->materials);
    cJSON *materials_array = (cJSON_GetObjectItem(self->root, "materials"));
    int materials_count = cJSON_GetArraySize(materials_array);

    for (int i = 0; i < materials_count; i++)
    {
        printf("parsing material %i\n", i);
        cJSON *material = cJSON_GetArrayItem(materials_array, i);
        GltfMaterial current = {
            .base = -1,
        };

        cJSON *pbr = cJSON_GetObjectItem(material, "pbrMetallicRoughness");

        current.metallic_fact = 1;
        current.rougness_fact = 1;

        current.metallic_roughness = -1;

        current.normal = -1;

        current.alpha = 1.0;
        current.emit = -1;
        current.normal_mul = 1;
        if (pbr != 0)
        {
            printf("has pbr\n");

            cJSON *base = cJSON_GetObjectItem(pbr, "baseColorTexture");
            cJSON *base_col = cJSON_GetObjectItem(pbr, "baseColorFactor");

            if (base != NULL)
            {
                int id = cJSON_GetObjectItem(base, "index")->valueint;

                printf("has base: %i\n", id);
                current.base = self->textures.data[id];
            }
            else if (base_col != NULL)
            {
                float r = cJSON_GetArrayItem(base_col, 0)->valuedouble;

                float g = cJSON_GetArrayItem(base_col, 1)->valuedouble;
                float b = cJSON_GetArrayItem(base_col, 2)->valuedouble;
                float a = cJSON_GetArrayItem(base_col, 3)->valuedouble;

                printf("has color %f %f %f %f\n", r, g, b, a);
                current.alpha = a;
                current.is_color = true;
                current.color = vec3$(r, g, b);
            }

            cJSON *metallic_roughness = cJSON_GetObjectItem(pbr, "metallicRoughnessTexture");
            if (metallic_roughness != NULL)
            {
                printf("has metallic roughness\n");
                int id = cJSON_GetObjectItem(metallic_roughness, "index")->valueint;
                current.metallic_roughness = self->textures.data[id];
            }
            cJSON *rougness_fact = cJSON_GetObjectItem(pbr, "roughnessFactor");
            if (rougness_fact != NULL)
            {
                current.rougness_fact = rougness_fact->valuedouble;

                printf("has metallic roughness factor %f \n", current.rougness_fact);
            }
            cJSON *metallic_fact = cJSON_GetObjectItem(pbr, "metallicFactor");
            if (metallic_fact != NULL)
            {
                current.metallic_fact = metallic_fact->valuedouble;

                printf("has metallic metal factor %f \n", current.metallic_fact);
            }
        }
        else
        {
            printf("no\n");
        }

        cJSON *normal = cJSON_GetObjectItem(material, "normalTexture");
        if (normal != NULL)
        {

            int id = cJSON_GetObjectItem(normal, "index")->valueint;
            current.normal = self->textures.data[id];

            cJSON *normal_scale = cJSON_GetObjectItem(normal, "scale");
            if (normal_scale)
            {
                current.normal_mul = normal_scale->valuedouble;
            }
        }
        cJSON *emit = cJSON_GetObjectItem(material, "emissiveTexture");
        if (emit != NULL)
        {
            int id = cJSON_GetObjectItem(emit, "index")->valueint;
            current.emit = self->textures.data[id];
        }

        else
        {
            current.emit = -1;
        }

        cJSON *emit_fact = cJSON_GetObjectItem(material, "emissiveFactor");
        if (emit_fact != NULL)
        {
            current.emissive_fact = vec3$(cJSON_GetArrayItem(emit_fact, 0)->valuedouble, cJSON_GetArrayItem(emit_fact, 1)->valuedouble, cJSON_GetArrayItem(emit_fact, 2)->valuedouble);

            cJSON *emissive_stren = cJSON_GetObjectItem(
                cJSON_GetObjectItem(
                    cJSON_GetObjectItem(material, "extensions"),
                    "KHR_materials_emissive_strength"),
                "emissiveStrength");

            if (emissive_stren != NULL)
            {
                current.emissive_fact = vec3_mul_val(current.emissive_fact, emissive_stren->valuedouble);
            }
        }

        Pbrt final = {
            .base = current.base,
            .is_color = current.is_color,
            .color = current.color,
            .normal = current.normal,
            .emit = current.emit,
            .roughness = current.metallic_roughness,
            .metallic_fact = current.metallic_fact,
            .rougness_fact = current.rougness_fact,
            .alpha = current.alpha,
            .normal_mul = current.normal_mul,
            .emmisive_fact = current.emissive_fact,
        };
        current.final = scene_push_pbrt(self->target, final);

        vec_push(&self->materials, current);
    }

    self->null_material_id = self->materials.length;

    Pbrt final = {
        .base = -1,
        .is_color = true,
        .color = vec3$(1, 1, 1),
        .normal = -1,
        .emit = -1,
        .roughness = -1,
        .metallic_fact = 0,
        .rougness_fact = 0.5,
        .alpha = 1.0,
        .normal_mul = 1,
        .emmisive_fact = vec3$(0, 0, 0),
    };

    GltfMaterial material = {
        .base = -1,
    };
    material.final = scene_push_pbrt(self->target, final);

    vec_push(&self->materials, material);
}
