/*#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;


void main() {
    outColor = vec4(fragColor, 1.0);
}*/
#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_spirv_intrinsics : enable
#extension GL_GOOGLE_include_directive : enable

#define M_PI 3.1415926535897932384626433832795

#define WORKGROUP_SIZE 32

struct Pixel
{
    vec4 value;
};

layout(std140, binding = 0) buffer buf
{
    Pixel image[];
};

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
}
ubo;

#include "hittable/hittable.comp"
#include "hittable/meshes.comp"
#include "utils/camera.comp"
#include "utils/random.comp"
#include "utils/ray.comp"
#include "utils/vec.comp"

#define PER_THREAD 1
layout(location = 0) in vec2 fragCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    const float window_width = (ubo.width);
    const float window_height = (ubo.height);

    int x = int(fragCoord.x * ubo.width);
    int y = int(fragCoord.y * ubo.height);

    vec2 cc = vec2(x, y);
    random_init(cc);
    camera cam = camera_init(ubo.camera_pos.xyz, vec3(ubo.camera_target.xyz), vec3(0, 1, 0), 0.1, 10.0);

    const uint pos = uint(ubo.width) * y + x;

    uint pos2 = uint(window_width) * ((uint(window_height) - (y)) + 1) + (x);
    uint pos3 = uint(window_width) * (((y))) + (x);

    vec4 prev = image[pos2].value;

    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
    for (int c = 0; c < PER_THREAD; c++)
    {

        float u = ((float(x) + random()) / (window_width - 1));
        float v = ((float(y) + random()) / (window_height - 1));

        Ray r = camera_ray(cam, u, v);
        r.inv_direction = vec3(1) / r.direction;
        color += ray_color(r, u, v, 4);
    }

    image[pos2].value = ((prev * (ubo.t * PER_THREAD)) + color) / ((ubo.t + 1) * PER_THREAD);

    outColor = image[pos3].value;
}
/*
int width = uint(ubo.width);
uint height = uint(ubo.height);
#define NO_DENOISE
void main()
{

    int ox = int(fragCoord.x * width);
    int oy = int(fragCoord.y * height);
    vec4 v = image[oy * width + ox].value;
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
*/
