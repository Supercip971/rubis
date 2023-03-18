#version 460 core

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 texCoord1;
layout(location = 4) in vec2 texCoord2;

#define M_PI 3.14159265f

#include "hittable/meshes.comp"

layout(std140, binding = 1) readonly uniform UniformBufferObject
{
    float width;
    float height;
    uint t;

    vec4 camera_pos;
    vec4 camera_target;
    vec4 camera_up;

    float aperture;
    float focus_disk;


    mat4 proj;
    mat4 view;
}
ubo;

layout( push_constant ) uniform constants
{
    int mesh_index;
    int material_index;
} PushConstants;


layout(location = 0) out vec3 fragColor;
void main() {

   vec4 t =  (inTangent);
   vec2 tc =  (texCoord1);
   vec2 tb =  (texCoord2);
    gl_Position = ubo.proj * ubo.view *vec4(inPosition.xyz, 1.0) ;

	gl_Position.y = -gl_Position.y;	
    PbrtTexture albedo_T = material_load_tex(int(PushConstants.material_index), 0);

    vec2 uv = tc;
    if(albedo_T.tid == 1)
    {
        uv = tb;
    }
    fragColor = material_tex_query(albedo_T, uv) * albedo_T.factor.xyz;
}