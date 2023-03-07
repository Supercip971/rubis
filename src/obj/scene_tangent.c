#include <obj/scene.h>
#include <thirdparty/mikktspace.h>
#include "math/vec3.h"
#include "obj/mesh.h"

#include <stdio.h>
typedef struct 
{
    Scene *scene;
    int mesh_id;
} TengantGenCtx ;

int scene_get_num_faces(const SMikkTSpaceContext *pContext)
{
    TengantGenCtx *ctx = pContext->m_pUserData;
    return mesh_count_faces(&ctx->scene->meshes.data[ctx->mesh_id]);
}

int scene_get_num_faces_vertices(const SMikkTSpaceContext *pContext, int iFace)
{
    (void)pContext;
    (void)iFace;
    return 3;
}


void scene_get_position(const SMikkTSpaceContext *pContext, float fvPosOut[], int iFace, int iVert)
{
    TengantGenCtx *ctx = pContext->m_pUserData;
    Mesh *mesh = &ctx->scene->meshes.data[ctx->mesh_id];
    SVertex vert = mesh_read_vertex(ctx->scene, mesh, iVert+ iFace * 3);

    fvPosOut[0] = vert.pos.x;
    fvPosOut[1] = vert.pos.y;
    fvPosOut[2] = vert.pos.z;
}

void scene_get_normal(const SMikkTSpaceContext *pContext, float fvNormOut[], int iFace, int iVert)
{
    TengantGenCtx *ctx = pContext->m_pUserData;
    Mesh *mesh = &ctx->scene->meshes.data[ctx->mesh_id];
    SVertex vert = mesh_read_vertex(ctx->scene, mesh, iVert+ iFace * 3);

    fvNormOut[0] = vert.normal.x;
    fvNormOut[1] = vert.normal.y;
    fvNormOut[2] = vert.normal.z;

}

int scene_texture_id_normal(Scene *scene, int material_d)
{
    PbrtMaterialImage img = scene_get_pbrt(scene, material_d + 3);

    if(img.id == -1)
    {
        return 0;
    }
    else {
        return img.tid;
    }
}

void scene_get_texCoord(const SMikkTSpaceContext *pContext, float fvTexOut[], int iFace, int iVert)
{
    TengantGenCtx *ctx = pContext->m_pUserData;
    Mesh *mesh = &ctx->scene->meshes.data[ctx->mesh_id];
    
    int offset = scene_texture_id_normal(ctx->scene, mesh->material.start) * 2;
    // see:  https://github.com/KhronosGroup/glTF-Sample-Models/issues/174
    SVertex vert = mesh_read_vertex(ctx->scene, mesh, iVert+ iFace * 3);

    if(offset == 0)
    {
        fvTexOut[0] = vert.tc1.x;
        fvTexOut[1] = vert.tc1.y;

        return;
    }

    fvTexOut[0] = vert.tc2.x;
    fvTexOut[1] = vert.tc2.y;

}

void scene_set_tangent_basic(const SMikkTSpaceContext *pContext, const float fvTangent[], float fSign, int iFace, int iVert)
{
    TengantGenCtx *ctx = pContext->m_pUserData;
    Mesh *mesh = &ctx->scene->meshes.data[ctx->mesh_id];

    SVertex vert = mesh_read_vertex(ctx->scene, mesh, iVert+ iFace * 3);
    vert.tangent.x = fvTangent[0];
    vert.tangent.y = fvTangent[1];
    vert.tangent.z = fvTangent[2];
    // see:  https://github.com/KhronosGroup/glTF-Sample-Models/issues/174
    vert.tangent._padding = -fSign;
    mesh_write_vertex(ctx->scene, mesh, iVert+ iFace * 3, vert);
}

bool scene_generate_tangent(Scene* self)
{   

    SMikkTSpaceInterface interface = {
        .m_setTSpaceBasic = scene_set_tangent_basic,
        .m_setTSpace = NULL,
        .m_getNormal = scene_get_normal,
        .m_getNumFaces = scene_get_num_faces,
        .m_getNumVerticesOfFace = scene_get_num_faces_vertices,
        .m_getPosition = scene_get_position,
        .m_getTexCoord = scene_get_texCoord,
    };


    SMikkTSpaceContext context = {
        .m_pInterface = &interface,
    };


    for(int i = 0; i < self->meshes.length; i++)
    {

        int count = mesh_count_faces(&self->meshes.data[i]);

        printf("generating tengant [%i:%i]...\n", i, count);
        TengantGenCtx ctx = {
            .scene = self,
            .mesh_id = i,
        };
        context.m_pUserData = &ctx;
        bool res = genTangSpaceDefault(&context);
        if(!res)
        {
            printf("unable to generate tengant for %i\n", i);
            return false;
        }
    }

    return true;

}
