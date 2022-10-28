#include <obj/scene.h>
#include <thirdparty/mikktspace.h>


int scene_get_num_faces(const SMikkTSpaceContext *pContext)
{
    Scene *scene = pContext->m_pUserData;
    return scene->meshes.length;
}

int scene_get_num_faces_vertices(const SMikkTSpaceContext *pContext, int iFace)
{
    (void)pContext;
    (void)iFace;
    return 3;
}


void scene_get_position(const SMikkTSpaceContext *pContext, float fvPosOut[], int iFace, int iVert)
{
    Scene *scene = pContext->m_pUserData;
    Mesh *mesh = &scene->meshes.data[iFace];
    fvPosOut[0] = scene->data.data[mesh->vertices.start + iVert].x;
    fvPosOut[1] = scene->data.data[mesh->vertices.start + iVert].y;
    fvPosOut[2] = scene->data.data[mesh->vertices.start + iVert].z;

}

void scene_get_normal(const SMikkTSpaceContext *pContext, float fvNormOut[], int iFace, int iVert)
{
    Scene *scene = pContext->m_pUserData;
    Mesh *mesh = &scene->meshes.data[iFace];
    fvNormOut[0] = scene->data.data[mesh->vertices.start + 5 + iVert].x;
    fvNormOut[1] = scene->data.data[mesh->vertices.start + 5 + iVert].y;
    fvNormOut[2] = scene->data.data[mesh->vertices.start + 5 + iVert].z;
}
void scene_get_texCoord(const SMikkTSpaceContext *pContext, float fvTexOut[], int iFace, int iVert)
{
    Scene *scene = pContext->m_pUserData;
    Mesh *mesh = &scene->meshes.data[iFace];
    

    switch(iVert)
    {
        case 0:
            fvTexOut[0] = scene->data.data[mesh->vertices.start + 3].x;
            fvTexOut[1] = scene->data.data[mesh->vertices.start + 3].y;
            break;
        case 1:
            fvTexOut[0] = scene->data.data[mesh->vertices.start + 3].z;
            fvTexOut[1] = scene->data.data[mesh->vertices.start + 4].x;
            break;
        case 2:
            fvTexOut[0] = scene->data.data[mesh->vertices.start + 4].y;
            fvTexOut[1] = scene->data.data[mesh->vertices.start + 4].z;
            break;
    }
}

void scene_set_tangent_basic(const SMikkTSpaceContext *pContext, const float fvTangent[], float fSign, int iFace, int iVert)
{
    Scene *scene = pContext->m_pUserData;
    Mesh *mesh = &scene->meshes.data[iFace];
    scene->data.data[mesh->vertices.start + 8 + iVert].x = fvTangent[0];
    scene->data.data[mesh->vertices.start + 8 + iVert].y = fvTangent[1];
    scene->data.data[mesh->vertices.start + 8 + iVert].z = fvTangent[2];
    scene->data.data[mesh->vertices.start + 8 + iVert]._padding = fSign;
}

bool scene_generate_tangent(Scene* self)
{
    void* ctx = self;

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
        .m_pUserData = ctx,
    };

    return genTangSpaceDefault(&context);
}
