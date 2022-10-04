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
layout(binding = 0, rgba32f) uniform image2D buf2;

layout(binding = 1) readonly uniform UniformBufferObject
{
    float width;
    float height;
    uint t;

    vec4 camera_pos;

    vec4 camera_target;
    vec4 camera_up;
    float aperture;
    float focus_disk;
    int denoise;
}
ubo;

layout(location = 0) in vec2 fragCoord;

layout(location = 0) out vec4 outColor;

uint width = uint(ubo.width);
uint height = uint(ubo.height);
#define NO_DENOISE
void main()
{
    
   
    int ox = int(fragCoord.x * width);
    int oy = int((1 - fragCoord.y) * height);
    vec4 v = image[(oy)*width + ox].value;
       outColor = vec4(v.x, v.y, v.z, 1.0f);
    //
    //    {
    //        vec4 color = vec4(0.0);
    //        float total = 0.0;
    //        vec4 center = min(image[oy * width + ox].value, vec4(1));
    //
    //        for (float x = -4.0; x <= 4.0; x += 1.0)
    //        {
    //            for (float y = -4.0; y <= 4.0; y += 1.0)
    //            {
    //                int pos = int(ox + x) + int(oy + y) * int(width);
    //
    //                vec4 s = min(image[pos].value, vec4(1));
    //                float weight = 1.0 - abs(dot(s.xyz - center.xyz, vec3(0.25)));
    //
    //                weight = pow(weight, 16);
    //                color += s * weight;
    //                total += weight;
    //            }
    //        }
    //
    //        outColor = (((color / total)) + center) / 2;
    //        // outColor = vec4(1);
    //    }
}