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
    fvPosOut[0] = ctx->scene->data.data[mesh->vertices.start + iVert + iFace * MESH_VERTICE_COUNT].x;
    fvPosOut[1] = ctx->scene->data.data[mesh->vertices.start + iVert + iFace * MESH_VERTICE_COUNT].y;
    fvPosOut[2] = ctx->scene->data.data[mesh->vertices.start + iVert + iFace * MESH_VERTICE_COUNT].z;

}

void scene_get_normal(const SMikkTSpaceContext *pContext, float fvNormOut[], int iFace, int iVert)
{
    TengantGenCtx *ctx = pContext->m_pUserData;
    Mesh *mesh = &ctx->scene->meshes.data[ctx->mesh_id];
    fvNormOut[0] = ctx->scene->data.data[mesh->vertices.start + iVert + 7 + iFace * MESH_VERTICE_COUNT].x;
    fvNormOut[1] = ctx->scene->data.data[mesh->vertices.start + iVert + 7 + iFace * MESH_VERTICE_COUNT].y;
    fvNormOut[2] = ctx->scene->data.data[mesh->vertices.start + iVert + 7 + iFace * MESH_VERTICE_COUNT].z;
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
    switch(iVert)
    {
        case 0:
            fvTexOut[0] = ctx->scene->data.data[mesh->vertices.start + 3 + offset + iFace * MESH_VERTICE_COUNT].x;
            fvTexOut[1] = ctx->scene->data.data[mesh->vertices.start + 3 + offset + iFace * MESH_VERTICE_COUNT].y;
            break;
        case 1:
            fvTexOut[0] = ctx->scene->data.data[mesh->vertices.start + 3 + offset + iFace * MESH_VERTICE_COUNT].z;
            fvTexOut[1] = ctx->scene->data.data[mesh->vertices.start + 4 + offset + iFace * MESH_VERTICE_COUNT].x;
            break;
        case 2:
            fvTexOut[0] = ctx->scene->data.data[mesh->vertices.start + 4 + offset + iFace * MESH_VERTICE_COUNT].y;
            fvTexOut[1] = ctx->scene->data.data[mesh->vertices.start + 4 + offset + iFace * MESH_VERTICE_COUNT].z;
            break;
    }
}

void scene_set_tangent_basic(const SMikkTSpaceContext *pContext, const float fvTangent[], float fSign, int iFace, int iVert)
{
    TengantGenCtx *ctx = pContext->m_pUserData;
    Mesh *mesh = &ctx->scene->meshes.data[ctx->mesh_id];
      // see:  https://github.com/KhronosGroup/glTF-Sample-Models/issues/174
    
    ctx->scene->data.data[mesh->vertices.start + 10 + iVert + iFace * MESH_VERTICE_COUNT].x = fvTangent[0];
    ctx->scene->data.data[mesh->vertices.start + 10 + iVert + iFace * MESH_VERTICE_COUNT].y = fvTangent[1];
    ctx->scene->data.data[mesh->vertices.start + 10 + iVert + iFace * MESH_VERTICE_COUNT].z = fvTangent[2];
    ctx->scene->data.data[mesh->vertices.start + 10 + iVert + iFace * MESH_VERTICE_COUNT]._padding = -fSign;
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
