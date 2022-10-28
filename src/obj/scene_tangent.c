#include <obj/scene.h>
#include <thirdparty/mikktspace.h>
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
    fvNormOut[0] = ctx->scene->data.data[mesh->vertices.start + iVert + 5+ iFace * MESH_VERTICE_COUNT].x;
    fvNormOut[1] = ctx->scene->data.data[mesh->vertices.start + iVert + 5+ iFace * MESH_VERTICE_COUNT].y;
    fvNormOut[2] = ctx->scene->data.data[mesh->vertices.start + iVert + 5+ iFace * MESH_VERTICE_COUNT].z;
}

void scene_get_texCoord(const SMikkTSpaceContext *pContext, float fvTexOut[], int iFace, int iVert)
{
    TengantGenCtx *ctx = pContext->m_pUserData;
    Mesh *mesh = &ctx->scene->meshes.data[ctx->mesh_id];
    

    switch(iVert)
    {
        case 0:
            fvTexOut[0] = ctx->scene->data.data[mesh->vertices.start + 3 + iFace * MESH_VERTICE_COUNT].x;
            fvTexOut[1] = ctx->scene->data.data[mesh->vertices.start + 3 + iFace * MESH_VERTICE_COUNT].y;
            break;
        case 1:
            fvTexOut[0] = ctx->scene->data.data[mesh->vertices.start + 3 + iFace * MESH_VERTICE_COUNT].z;
            fvTexOut[1] = ctx->scene->data.data[mesh->vertices.start + 4 + iFace * MESH_VERTICE_COUNT].x;
            break;
        case 2:
            fvTexOut[0] = ctx->scene->data.data[mesh->vertices.start + 4 + iFace * MESH_VERTICE_COUNT].y;
            fvTexOut[1] = ctx->scene->data.data[mesh->vertices.start + 4 + iFace * MESH_VERTICE_COUNT].z;
            break;
    }
}

void scene_set_tangent_basic(const SMikkTSpaceContext *pContext, const float fvTangent[], float fSign, int iFace, int iVert)
{
    TengantGenCtx *ctx = pContext->m_pUserData;
    Mesh *mesh = &ctx->scene->meshes.data[ctx->mesh_id];
    
    ctx->scene->data.data[mesh->vertices.start + 8 + iVert + iFace * MESH_VERTICE_COUNT].x = fvTangent[0];
    ctx->scene->data.data[mesh->vertices.start + 8 + iVert + iFace * MESH_VERTICE_COUNT].y = fvTangent[1];
    ctx->scene->data.data[mesh->vertices.start + 8 + iVert + iFace * MESH_VERTICE_COUNT].z = fvTangent[2];
    ctx->scene->data.data[mesh->vertices.start + 8 + iVert + iFace * MESH_VERTICE_COUNT]._padding = fSign;
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

        printf("generating tengant [%i]...\n", i);
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
