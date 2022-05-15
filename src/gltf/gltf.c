#include "gltf.h"
#include <cjson/cJSON.h>
#include <cjson/cJSON_Utils.h>
#include <gltf/accessor.h>
#include <stdio.h>

typedef struct __attribute__((packed))
{
    float x, y, z;
} GltfVec3;

#define gltvec2vec(v) vec3$(v.x, v.y, v.z)

bool parse_gltf_mesh(cJSON *gltf, cJSON *node, GltfChunkHeader *binary, Scene *target)
{
    cJSON *mesh_primitives_array = cJSON_GetObjectItem(node, "primitives");
    int primitive_count = cJSON_GetArraySize(mesh_primitives_array);

    for (int i = 0; i < primitive_count; i++)
    {
        cJSON *primitive = cJSON_GetArrayItem(mesh_primitives_array, i);
        cJSON *attributes = cJSON_GetObjectItem(primitive, "attributes");
        GltfAccessorPtr indicies = gltf_read_accessor(gltf, cJSON_GetObjectItem(primitive, "indices")->valueint, binary);

        GltfAccessorPtr position = gltf_read_accessor(gltf, cJSON_GetObjectItem(attributes, "POSITION")->valueint, binary);
        for (int j = 0; j < indicies.count; j += 3)
        {
            GltfVec3 *v = position.data;
            uint16_t *idx = indicies.data;
            idx += j;
            scene_push_tri(target,
                           vec3_mul_val(gltvec2vec(v[idx[0]]), 0.5),
                           vec3_mul_val(gltvec2vec(v[idx[1]]), 0.5),
                           vec3_mul_val(gltvec2vec(v[idx[2]]), 0.5),
                           scene_push_lambertian(target, vec3$(0.5, 0.5, 0.5)));
        }
    }

    return true;
}
bool parse_gltf_node(cJSON *gltf, cJSON *node, GltfChunkHeader *binary, Scene *target)
{
    // not support for modification/rotation/scale !FOR THE MOMENT!
    cJSON *object_mesh_id = (cJSON_GetObjectItem(node, "mesh"));

    cJSON *mesh_element = cJSON_GetArrayItem(cJSON_GetObjectItem(gltf, "meshes"), object_mesh_id->valueint);

    bool mesh = parse_gltf_mesh(gltf, mesh_element, binary, target);
    if (!mesh)
    {
        return false;
    }
    return true;
}
// maybe this should be split in 2 function
bool parse_gltf_scene(cJSON *gltf, GltfChunkHeader *binary, Scene *target)
{
    cJSON *scene_id = (cJSON_GetObjectItem(gltf, "scene"));

    cJSON *scene_element = cJSON_GetArrayItem(cJSON_GetObjectItem(gltf, "scenes"), scene_id->valueint); // root->scenes[scene_id]
    cJSON *scene_nodes_id_array = cJSON_GetObjectItem(scene_element, "nodes");                          // root->scenes[scene_id].nodes
    cJSON *gltf_nodes_array = cJSON_GetObjectItem(gltf, "nodes");                                       // root->node

    int node_count = cJSON_GetArraySize(scene_nodes_id_array);
    for (int i = 0; i < node_count; i++)
    {
        int node_id = cJSON_GetArrayItem(scene_nodes_id_array, i)->valueint;
        bool res = parse_gltf_node(gltf, cJSON_GetArrayItem(gltf_nodes_array, node_id), binary, target); // root->node[i]
        if (!res)
        {
            return false;
        }
    }
    return true;
}
bool parse_gltf(void *data, Scene *target)
{
    GltfHeader *header = data;

    if (data == NULL || header->magic != GLTF_MAGIC)
    {
        printf("can't load gltf: invalid magic value\n");
        return false;
    }
    if (header->version != 2)
    {
        printf("file use a not supported gltf version %i ! (we currently only support version 2)\n", header->version);
        return false;
    }
    GltfChunkHeader *json_header = data + 12;
    GltfChunkHeader *binary_header = data + 12 + sizeof(GltfChunkHeader) + json_header->len;

    cJSON *json = cJSON_Parse((const char *)json_header->data);
    if (json == NULL)
    {
        // this is a weird case
        const char *error = cJSON_GetErrorPtr();
        if (error != NULL)
        {
            printf("error while parsing gltf json: %s \n", error);
            return 0;
        }
    }

    return parse_gltf_scene(json, binary_header, target);
}
