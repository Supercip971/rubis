/*#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;


void main() {
    outColor = vec4(fragColor, 1.0);
}*/
#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Pixel
{
    vec4 value;
};

layout(std140, binding = 4) readonly buffer buf
{
    Pixel image[];
};

layout(binding = 1) readonly uniform UniformBufferObject
{
    float width;
    float height;
    uint t;

    vec3 camera_pos;

    vec3 camera_target;
    vec3 camera_up;
}
ubo;

layout(location = 0) in vec2 fragCoord;

layout(location = 0) out vec4 outColor;

uint width = uint(ubo.width);
uint height = uint(ubo.height);

void main()
{

    int x = int(fragCoord.x * width);
    int y = int(fragCoord.y * height);

    vec4 v = image[y * width + x].value;
    outColor = vec4(v.x, v.y, v.z, 1.0f);
}