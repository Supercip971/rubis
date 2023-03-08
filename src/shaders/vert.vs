#version 460 core

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 texCoord1;
layout(location = 4) in vec2 texCoord2;


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


layout(location = 0) out vec3 fragColor;
void main() {

   vec4 t =  (inTangent);
   vec2 tc =  (texCoord1);
   vec2 tb =  (texCoord2);
    gl_Position = ubo.proj * ubo.view *vec4(inPosition.xyz, 1.0) ;

	gl_Position.y = -gl_Position.y;	
    fragColor = inColor.xyz;
}