#version 460 core

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 texCoord1;
layout(location = 4) in vec2 texCoord2;

#extension GL_EXT_nonuniform_qualifier : require
#define M_PI 3.14159265f

layout(std140, push_constant ) uniform UniformBufferObject
{
    float width;
    float height;
    uint t;

    vec4 camera_pos;
    vec4 camera_target;
    vec4 camera_up;

    float aperture;
    float focus_disk;


    uint bounce_limit;
    uint scale;

    uint use_fsr;
    int mesh_index;
    int material_index;
} ubo;
#include "hittable/meshes.comp"



layout(location = 0) out vec3 fragColor;

mat4 matrix_lookat(vec3 p, vec3 center, vec3 up)
{
    vec3 f = normalize(center - p);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    mat4 m = mat4(1.0);

    m[0][0] = s.x;
    m[1][0] = s.y;
    m[2][0] = s.z;
    m[0][1] = u.x;
    m[1][1] = u.y;
    m[2][1] = u.z;
    m[0][2] = -f.x;
    m[1][2] = -f.y;
    m[2][2] = -f.z;
    m[3][3] = 1.0f;

    m[3][0] = -dot(s, p.xyz);
    m[3][1] = -dot(u, p.xyz);
    m[3][2] = dot(f, p.xyz);

    return m;
}

mat4 matrix_perspective(float fov_deg, float ratio, float near, float far)
{
    float fov_rad = fov_deg * M_PI / 180.0f;
    float f = 1.0f / tan(fov_rad / 2.0f);
    mat4 m = mat4(1.0);
    m[0][0] = f / ratio;
    m[1][1] = f;
    m[2][2] = (far + near) / (near - far);
    m[2][3] = -1.0f;
    m[3][2] = (2.0f * far * near) / (near - far);
    m[3][3] = 1.0;
    return m;
}
void main() {

    vec4 t =  (inTangent);
    vec2 tc =  (texCoord1);
    vec2 tb =  (texCoord2);
    mat4 view_m = matrix_lookat(ubo.camera_pos.xyz, ubo.camera_target.xyz, ubo.camera_up.xyz);

    mat4 proj_m = matrix_perspective( ubo.focus_disk, ubo.width / ubo.height, 0.1f, 1000.0f);
    gl_Position = proj_m * view_m *vec4(inPosition.xyz, 1.0) ;

	gl_Position.y = -gl_Position.y;	
    PbrtTexture albedo_T = material_load_tex(int(ubo.material_index), 0);

    vec2 uv = tc;
    if(albedo_T.tid == 1)
    {
        uv = tb;
    }
    fragColor = material_tex_query(albedo_T, uv) * albedo_T.factor.xyz;
}