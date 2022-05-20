
#include <gltf/accessor.h>
#include <gltf/materials.h>
#include <stdio.h>

void gltf_materials_parse(GltfCtx *self)
{
    vec_init(&self->materials);
    cJSON *materials_array = (cJSON_GetObjectItem(self->root, "materials"));
    int materials_count = cJSON_GetArraySize(materials_array);

    for (int i = 0; i < materials_count; i++)
    {
        cJSON *material = cJSON_GetArrayItem(materials_array, i);
        GltfMaterial current = {};

        cJSON *pbr = cJSON_GetObjectItem(material, "pbrMetallicRoughness");
        if (pbr != 0)
        {
            printf("has pbr\n");

            cJSON *base = cJSON_GetObjectItem(pbr, "baseColorTexture");
            if (base != NULL)
            {
                printf("has base\n");
                int id = cJSON_GetObjectItem(base, "index")->valueint;
                current.base = self->textures.data[id];
            }

            cJSON *metallic_roughness = cJSON_GetObjectItem(material, "metallicRoughnessTexture");
            if (metallic_roughness != NULL)
            {
                int id = cJSON_GetObjectItem(metallic_roughness, "index")->valueint;
                current.base = self->textures.data[id];
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
        }

        current.final = scene_push_pbrt(self->target, current.normal, current.base, current.metallic_roughness);

        vec_push(&self->materials, current);
    }
}
