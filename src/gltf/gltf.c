#include "gltf.h"
#include <gltf/accessor.h>
#include <gltf/materials.h>
#include <gltf/textures.h>
#include <stdio.h>

typedef struct __attribute__((packed))
{
    float x, y, z;
} GltfVec3;
typedef struct __attribute__((packed))
{
    float x, y;
} GltfVec2;

#define gltvec2vec(v) vec3$(v.x, v.y, v.z)

bool parse_gltf_mesh(GltfCtx *self, cJSON *node)
{
    cJSON *mesh_primitives_array = cJSON_GetObjectItem(node, "primitives");
    int primitive_count = cJSON_GetArraySize(mesh_primitives_array);
    for (int i = 0; i < primitive_count; i++)
    {
        cJSON *primitive = cJSON_GetArrayItem(mesh_primitives_array, i);
        cJSON *attributes = cJSON_GetObjectItem(primitive, "attributes");
        GltfAccessorPtr indicies = gltf_read_accessor(self, cJSON_GetObjectItem(primitive, "indices")->valueint);
        int material = cJSON_GetObjectItem(primitive, "material")->valueint;

        printf("primitives: %i %i \n", cJSON_GetObjectItem(primitive, "indices")->valueint, i);
        printf("pos: %i\n", cJSON_GetObjectItem(attributes, "POSITION")->valueint);
        GltfAccessorPtr position = gltf_read_accessor(self, cJSON_GetObjectItem(attributes, "POSITION")->valueint);
        GltfAccessorPtr texcoords = gltf_read_accessor(self, cJSON_GetObjectItem(attributes, "TEXCOORD_0")->valueint);

        for (int j = 0; j < indicies.count; j += 3)
        {
            GltfVec3 *v = position.view.data;
            GltfVec2 *gltftex = texcoords.view.data;

            uint16_t *idx = indicies.view.data;
            idx += j;
            Triangle final = {
                .pa = (gltvec2vec(v[idx[0]])),
                .pb = (gltvec2vec(v[idx[1]])),
                .pc = (gltvec2vec(v[idx[2]])),
            };

            final.tex_pos[0] = (TriangleTexPos){gltftex[idx[0]].x, gltftex[idx[0]].y};
            final.tex_pos[1] = (TriangleTexPos){gltftex[idx[1]].x, gltftex[idx[1]].y};
            final.tex_pos[2] = (TriangleTexPos){gltftex[idx[2]].x, gltftex[idx[2]].y};

            scene_push_tri2(self->target, final, self->materials.data[material].final);
        }
    }

    return true;
}
bool parse_gltf_node(GltfCtx *self, cJSON *node);

bool parse_gltf_node_misc(GltfCtx *self, cJSON *node)
{
    cJSON *childs = (cJSON_GetObjectItem(node, "children"));
    int child_count = cJSON_GetArraySize(childs);
    cJSON *gltf_nodes_array = cJSON_GetObjectItem(self->root, "nodes"); // root->node

    for (int i = 0; i < child_count; i++)
    {
        cJSON *child_id = cJSON_GetArrayItem(childs, i);
        if (!parse_gltf_node(self, cJSON_GetArrayItem(gltf_nodes_array, child_id->valueint)))
        {
            return false;
        }
    }
    return true;
}
bool parse_gltf_node(GltfCtx *self, cJSON *node)
{
    // not support for modification/rotation/scale !FOR THE MOMENT!
    cJSON *object_mesh_id = (cJSON_GetObjectItem(node, "mesh"));

    if (object_mesh_id == NULL)
    {

        return parse_gltf_node_misc(self, node);
    }

    cJSON *mesh_element = cJSON_GetArrayItem(cJSON_GetObjectItem(self->root, "meshes"), object_mesh_id->valueint);

    bool mesh = parse_gltf_mesh(self, mesh_element);
    if (!mesh)
    {
        return false;
    }
    return true;
}

// maybe this should be split in 2 function
bool parse_gltf_scene(GltfCtx *ctx)
{
    cJSON *scene_id = cJSON_GetObjectItem(ctx->root, "scene");

    cJSON *scene_element = cJSON_GetArrayItem(cJSON_GetObjectItem(ctx->root, "scenes"), scene_id->valueint); // root->scenes[scene_id]
    cJSON *scene_nodes_id_array = cJSON_GetObjectItem(scene_element, "nodes");                               // root->scenes[scene_id].nodes
    cJSON *gltf_nodes_array = cJSON_GetObjectItem(ctx->root, "nodes");                                       // root->node

    int node_count = cJSON_GetArraySize(scene_nodes_id_array);
    for (int i = 0; i < node_count; i++)
    {
        int node_id = cJSON_GetArrayItem(scene_nodes_id_array, i)->valueint;
        bool res = parse_gltf_node(ctx, cJSON_GetArrayItem(gltf_nodes_array, node_id)); // root->node[i]
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

    GltfCtx ctx = {
        .binary = binary_header,
        .data = data,
        .root = json,
        .target = target,
    };

    gltf_textures_parse(&ctx);

    gltf_materials_parse(&ctx);

    return parse_gltf_scene(&ctx);
}
