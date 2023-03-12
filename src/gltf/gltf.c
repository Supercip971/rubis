#include <cjson/cJSON.h>
#include <gltf/accessor.h>
#include <gltf/materials.h>
#include <gltf/textures.h>
#include <math/mat4.h>
#include <stdio.h>
#include "gltf.h"
#include "math/vec3.h"
#include "obj/mesh.h"
#include "obj/scene.h"

typedef struct __attribute__((packed))
{
    float x, y, z, w;
} GltfVec4;

typedef struct __attribute__((packed))
{
    float x, y, z;
} GltfVec3;
typedef struct __attribute__((packed))
{
    float x, y;
} GltfVec2;

#define gltvec2vec(v) vec3$(v.x, v.y, v.z)
#define gltvec42vec(v)     \
    (Vec3)                 \
    {                      \
        v.x, v.y, v.z, v.w \
    }

bool parse_gltf_mesh(GltfCtx *self, cJSON *node, Matrix4x4 transform)
{
    cJSON *mesh_primitives_array = cJSON_GetObjectItem(node, "primitives");

    int primitive_count = cJSON_GetArraySize(mesh_primitives_array);
    bool has_tangent_all = false;
    for (int i = 0; i < primitive_count; i++)
    {
        bool has_normal = false;
        // as each primitive can have different material / texture, we parse them as different meshes
        cJSON *primitive = cJSON_GetArrayItem(mesh_primitives_array, i);
        cJSON *attributes = cJSON_GetObjectItem(primitive, "attributes");
        GltfAccessorPtr indicies = gltf_read_accessor(self, cJSON_GetObjectItem(primitive, "indices")->valueint);
        int material = -1;
        MeshCreation mesh_creation;
        if (cJSON_GetObjectItem(primitive, "material") != NULL)
        {

            material = cJSON_GetObjectItem(primitive, "material")->valueint;
        }
        else
        {
            material = self->null_material_id;
        }
        bool has_tangent = cJSON_GetObjectItem(attributes, "TANGENT") != NULL;
        if (has_tangent)
        {
            has_tangent_all = true;
        }

        mesh_creation = scene_start_mesh(self->target, self->materials.data[material].final);

        printf("primitives: %i (%i) %i \n", cJSON_GetObjectItem(primitive, "indices")->valueint, indicies.count, i);
        printf("pos: %i\n", cJSON_GetObjectItem(attributes, "POSITION")->valueint);
        GltfAccessorPtr position = gltf_read_accessor(self, cJSON_GetObjectItem(attributes, "POSITION")->valueint);
        bool has_texcoord1 = cJSON_GetObjectItem(attributes, "TEXCOORD_0") != NULL;
        GltfAccessorPtr texcoords1;
        if (has_texcoord1)
        {
            texcoords1 = gltf_read_accessor(self, cJSON_GetObjectItem(attributes, "TEXCOORD_0")->valueint);
        }

        bool has_texcoord2 = cJSON_GetObjectItem(attributes, "TEXCOORD_1") != NULL;
        GltfAccessorPtr texcoords2;
        if (has_texcoord2)
        {

            texcoords2 = gltf_read_accessor(self, cJSON_GetObjectItem(attributes, "TEXCOORD_1")->valueint);
        }

        GltfAccessorPtr normals;
        if (cJSON_GetObjectItem(attributes, "NORMAL") == NULL)
        {
            has_normal = false;
        }
        else
        {
            has_normal = true;
            normals = gltf_read_accessor(self, cJSON_GetObjectItem(attributes, "NORMAL")->valueint);
        }

        GltfAccessorPtr tangents;
        if (has_tangent)
        {
            tangents = gltf_read_accessor(self, cJSON_GetObjectItem(attributes, "TANGENT")->valueint);
        }
        for (int j = 0; j < indicies.count; j += 3)
        {
            GltfVec3 *v = position.view.data;

            if (position.componen_type != GLTF_COMP_FLOAT)
            {
                printf("not supported position type: %i \n", position.componen_type);
                abort();
            }
            if (has_normal && normals.componen_type != GLTF_COMP_FLOAT)
            {
                printf("not supported normal type: %i \n", position.componen_type);
                abort();
            }

            GltfVec3 *n = normals.view.data;
            GltfVec4 *t = tangents.view.data;

            int idx0 = 0, idx1 = 0, idx2 = 0;

            if (indicies.componen_type == GLTF_COMP_U16)
            {
                uint16_t *idx = indicies.view.data;
                idx += j;
                idx0 = idx[0];
                idx1 = idx[1];
                idx2 = idx[2];
            }
            else if (indicies.componen_type == GLTF_COMP_U32)
            {
                uint32_t *idx = indicies.view.data;
                idx += j;
                idx0 = idx[0];
                idx1 = idx[1];
                idx2 = idx[2];
            }
            else
            {
                printf("invalid type: %i\n", indicies.componen_type);
                abort();
            }
            Triangle final = {
                .a.pos = matrix_apply_point_ret(&transform, gltvec2vec(v[idx0])),
                .b.pos = matrix_apply_point_ret(&transform, gltvec2vec(v[idx1])),
                .c.pos = matrix_apply_point_ret(&transform, gltvec2vec(v[idx2])),
            };

            if (has_normal)
            {
                final.a.normal = matrix_apply_vector_ret(&transform, gltvec2vec(n[idx0]));
                final.b.normal = matrix_apply_vector_ret(&transform, gltvec2vec(n[idx1]));
                final.c.normal = matrix_apply_vector_ret(&transform, gltvec2vec(n[idx2]));
                final.a.normal._padding = 1;
            }
            else
            {
                final.a.normal._padding = 0;
            }
            if (has_tangent)
            {
                final.a.tangent = matrix_apply_vector_ret(&transform, gltvec42vec(t[idx0]));
                final.b.tangent = matrix_apply_vector_ret(&transform, gltvec42vec(t[idx1]));
                final.c.tangent = matrix_apply_vector_ret(&transform, gltvec42vec(t[idx2]));
            }

            if (has_texcoord1)
            {
                if (texcoords1.componen_type != GLTF_COMP_FLOAT && texcoords1.type != GLTF_VEC2)
                {
                    printf("not supported texcoord type: %i \n", texcoords1.componen_type);
                    abort();
                }

                GltfVec2 *gltftex = texcoords1.view.data;
                final.a.tc1 = (TriangleTexPos){gltftex[idx0].x, gltftex[idx0].y};
                final.b.tc1 = (TriangleTexPos){gltftex[idx1].x, gltftex[idx1].y};
                final.c.tc1 = (TriangleTexPos){gltftex[idx2].x, gltftex[idx2].y};
            }
            if (has_texcoord2)
            {
                if (texcoords2.componen_type != GLTF_COMP_FLOAT && texcoords2.type != GLTF_VEC2)
                {
                    printf("not supported texcoord type: %i \n", texcoords2.componen_type);
                    abort();
                }

                GltfVec2 *gltftex = texcoords2.view.data;
                final.a.tc2 = (TriangleTexPos){gltftex[idx0].x, gltftex[idx0].y};
                final.b.tc2 = (TriangleTexPos){gltftex[idx1].x, gltftex[idx1].y};
                final.c.tc2 = (TriangleTexPos){gltftex[idx2].x, gltftex[idx2].y};
            }
            mesh_push_triangle(&mesh_creation, final);
        }
        mesh_creation.has_tangent = has_tangent_all;
        mesh_creation.mesh.type = (has_tangent_all) ? -1 : 2;
        mesh_gen_normals_if_needed(&mesh_creation);
        printf("end: %i\n", self->target->meshes.length);;
        scene_end_mesh(self->target, &mesh_creation);
    }

    return true;
}
bool parse_gltf_node(GltfCtx *self, cJSON *node, Matrix4x4 transform);
bool parse_gltf_camera(GltfCtx *self, cJSON *node, Matrix4x4 transform)
{
    cJSON *gltf_cameras_array = cJSON_GetObjectItem(self->root, "cameras"); // root->cameras

    int camera_id = cJSON_GetObjectItem(node, "camera")->valueint;

    cJSON *camera = cJSON_GetArrayItem(gltf_cameras_array, camera_id);

    if (cJSON_GetObjectItem(camera, "perspective") != NULL)
    {
        if (cJSON_GetObjectItem(camera, "yfov") != NULL)
        {

            self->target->camera_fov = cJSON_GetObjectItem(camera, "yfov")->valuedouble;
        }

        printf("VALID CAMERA\n");

        self->target->camera_transform = transform;
    }

    return true;
}
bool parse_gltf_node_family(GltfCtx *self, cJSON *node, Matrix4x4 transform)
{
    cJSON *childs = (cJSON_GetObjectItem(node, "children"));

    int child_count = cJSON_GetArraySize(childs);
    cJSON *gltf_nodes_array = cJSON_GetObjectItem(self->root, "nodes"); // root->node

    for (int i = 0; i < child_count; i++)
    {
        cJSON *child_id = cJSON_GetArrayItem(childs, i);
        if (!parse_gltf_node(self, cJSON_GetArrayItem(gltf_nodes_array, child_id->valueint), transform))
        {
            return false;
        }
    }
    return true;
}
bool parse_gltf_node_other(GltfCtx *self, cJSON *node, Matrix4x4 transform)
{
    cJSON *childs = (cJSON_GetObjectItem(node, "children"));

    if (childs == NULL && cJSON_GetObjectItem(node, "camera"))
    {
        printf("HAS CAMERA\n");
        return parse_gltf_camera(self, node, transform);
    }
    else if (childs != NULL)
    {
        parse_gltf_node_family(self, node, transform);
    }
    return true;
}

void parse_gltf_transforms(cJSON *node, Matrix4x4 *result)
{
    Matrix4x4 self_transform;
    Matrix4x4 translation;
    Matrix4x4 rotation;
    Matrix4x4 scale;

    create_matrix_identity(&translation);
    create_matrix_identity(&rotation);
    create_matrix_identity(&scale);

    cJSON *rotation_node = cJSON_GetObjectItem(node, "rotation");
    cJSON *translation_node = cJSON_GetObjectItem(node, "translation");
    cJSON *scale_node = cJSON_GetObjectItem(node, "scale");

    if (rotation_node)
    {
        double x = cJSON_GetArrayItem(rotation_node, 0)->valuedouble;
        double y = cJSON_GetArrayItem(rotation_node, 1)->valuedouble;
        double z = cJSON_GetArrayItem(rotation_node, 2)->valuedouble;
        double w = cJSON_GetArrayItem(rotation_node, 3)->valuedouble;

        create_matrix_rotate_q(&rotation, x, y, z, w);
    }
    if (translation_node)
    {
        double x = cJSON_GetArrayItem(translation_node, 0)->valuedouble;

        double y = cJSON_GetArrayItem(translation_node, 1)->valuedouble;
        double z = cJSON_GetArrayItem(translation_node, 2)->valuedouble;

        create_matrix_translate(&translation, x, y, z);
    }
    if (scale_node)
    {
        double x = cJSON_GetArrayItem(scale_node, 0)->valuedouble;

        double y = cJSON_GetArrayItem(scale_node, 1)->valuedouble;
        double z = cJSON_GetArrayItem(scale_node, 2)->valuedouble;

        create_matrix_scale(&scale, x, y, z);
    }

    // T * R * s
    create_matrix_identity(&self_transform);

    matrix_multiply(&self_transform, &translation, &self_transform);
    matrix_multiply(&self_transform, &scale, &self_transform);
    matrix_multiply(&self_transform, &rotation, &self_transform);

    *result = self_transform;
}

bool parse_gltf_node(GltfCtx *self, cJSON *node, Matrix4x4 transform)
{
    cJSON *object_mesh_id = (cJSON_GetObjectItem(node, "mesh"));

    Matrix4x4 self_transform;
    parse_gltf_transforms(node, &self_transform);
    Matrix4x4 res;
    matrix_multiply(&transform, &self_transform, &res);
    if (object_mesh_id == NULL)
    {

        return parse_gltf_node_other(self, node, res);
    }

    cJSON *mesh_element = cJSON_GetArrayItem(cJSON_GetObjectItem(self->root, "meshes"), object_mesh_id->valueint);
    const char *name = cJSON_GetStringValue(cJSON_GetObjectItem(mesh_element, "name"));
    if (name == NULL)
    {
        name = "unnamed";
    }
    printf("parsing mesh %s | %i\n", name, object_mesh_id->valueint);

    bool mesh = parse_gltf_mesh(self, mesh_element, res);
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
        Matrix4x4 m;
        create_matrix_identity(&m);

        bool res = parse_gltf_node(ctx, cJSON_GetArrayItem(gltf_nodes_array, node_id), m); // root->node[i]
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

    bool v = parse_gltf_scene(&ctx);

    if (!v)
    {
        return v;
    }

    v = scene_generate_tangent(ctx.target);
    if (!v)
    {
        return v;
    }
    return true;
}
