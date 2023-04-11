/*#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;


void main() {
    outColor = vec4(fragColor, 1.0);
}*/
#version 450

#extension GL_ARB_separate_shader_objects : enable
#define A_GPU 1
#define A_GLSL 1

struct Pixel
{
    vec4 value;
};

layout(binding = 3) uniform sampler2D buf;
layout(std140, push_constant) uniform UniformBufferObject
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
}
ubo;

uint width = uint(ubo.width);
uint height = uint(ubo.height);
layout(location = 0) in vec2 fragCoord;

layout(location = 0) out vec4 outColor;

vec2 image_uv(vec2 uv)
{

    float u = uv.x / ubo.scale;
    float v = ((1.0 - uv.y) / ubo.scale);
    return (vec2(u, v));
}

#define INV_SQRT_OF_2PI 0.39894228040143267793994605993439 // 1.0/SQRT_OF_2PI
#define INV_PI 0.31830988618379067153776752674503




vec4 image_value(vec2 uv)
{
    return (texture(buf, image_uv(uv)));
}
const float kernel[25] = {
    1.0 / 256.0, 1.0 / 64.0, 3.0 / 128.0, 1.0 / 64.0, 1.0 / 256.0,
    1.0 / 64.0, 1.0 / 16.0, 3.0 / 32.0, 1.0 / 16.0, 1.0 / 64.0,
    3.0 / 128.0, 3.0 / 32.0, 9.0 / 64.0, 3.0 / 32.0, 3.0 / 128.0,
    1.0 / 64.0, 1.0 / 16.0, 3.0 / 32.0, 1.0 / 16.0, 1.0 / 64.0,
    1.0 / 256.0, 1.0 / 64.0, 3.0 / 128.0, 1.0 / 64.0, 1.0 / 256.0};

struct raytraceInfo
{
    vec4 albedo;
    vec4 normal;
    vec4 position;
};
layout(set = 0, binding = 9, std140) readonly buffer info
{
    raytraceInfo infos[];
};

raytraceInfo read_pixel(float x, float y)
{
    const float height = float(ubo.height) / float(ubo.scale);
    const float width = float(ubo.width) / float(ubo.scale);
    return infos[uint(x * width) + uint(uint(height * y) * ubo.width)];
}
vec3 get_pixel_watrous(vec2 uv)
{
    vec3 final_color = vec3(0.0);
    vec4 sum = vec4(0.0);
    vec2 step = vec2(1. / width, 1. / height); // resolution

    vec2 ruv = vec2(uv);

    raytraceInfo pix = read_pixel(ruv.x, 1-ruv.y);
    vec4 cval = vec4(pix.albedo.xyz, 1.0);
    vec4 nval = vec4(pix.normal.xyz, 1.0);
    vec4 pval = vec4(pix.position.xyz, 1.0);
    float cum_w = 0.0;

    float n_phi = 0.3;
    float c_phi = 0.2;
    float stepwidth = 2;
    float p_phi = 0.3;

  //  for(float stepwidth = 1.0f; stepwidth < 16; stepwidth*= 2.0f)
  //  {

    for (int i = 0; i < 25; i++)
    {

        vec2 offset = vec2((i % 5) - 2, (i / 5) - 2) * stepwidth;
        vec2 coord = uv + (offset / vec2(width, height));

        ruv = vec2(coord);
        raytraceInfo npix = read_pixel(ruv.x, 1-ruv.y);
    
        vec4 ctmp = vec4(image_value(ruv).rgb, 1.0);
        vec4 col = vec4(npix.albedo.rgb, 1.0);


        vec4 t = cval - col;
        float dist2 = dot(t, t);
        float c_w = min(exp(-(dist2) / c_phi), 1.0);

        vec4 ntmp = vec4(npix.normal.xyz, 1.0);
        t = nval - ntmp;
        dist2 = max(dot(t, t) / (stepwidth * stepwidth), 0.0);
        float n_w = min(exp(-(dist2) / n_phi), 1.0);

        vec4 ptmp = vec4(npix.position.xyz, 1.0);
        t = pval - ptmp;
        dist2 = dot(t, t);
        float p_w = min(exp(-(dist2) / p_phi), 1.0);
        float weight = c_w * n_w * p_w;
        sum += ctmp * weight * kernel[i];
        cum_w += weight * kernel[i];
        // ----

      //  coord.x = clamp(coord.x, 0.0, 1.0);
      //  coord.y = clamp(coord.y, 0.0, 1.0);
      //  vec3 color = image_value(coord).rgb;
      //  color *= kernel[i];
      //  final_color += color;
  //  }

    }
    return (sum / cum_w).rgb;
}
#include "./fsr/ffx_a.comp"

AF4 FsrEasuRF(AF2 p)
{


    AF4 c = (textureGather(buf, (p / ubo.scale), 0));
    return c;
}
AF4 FsrEasuGF(AF2 p)
{
    AF4 c = (textureGather(buf, (p / ubo.scale), 1));
    return c;
}

AF4 FsrEasuBF(AF2 p)
{
    AF4 c = (textureGather(buf, (p / ubo.scale), 2));
    return c;
}
vec4 CurrFilter_easu(AU2 pos, uvec4 bias[4]);

uvec4 rbias[4];
AF4 FsrRcasLoadF(ASU2 p)
{

   // AF4 c = get_pixel_watrous(p.x, p.y);
    return CurrFilter_easu(AU2(p.x, p.y), rbias);
}
void FsrRcasInputF(inout AF1 r, inout AF1 g, inout AF1 b) {}

#define FSR_EASU_H 1
#define FSR_RCAS_H 1
#define FSR_RCAS_DENOISE 1
#define FSR_EASU_F 1
#define FSR_RCAS_F 1
// declare input callbacks
#include "./fsr/ffx_fsr1.comp"
vec4 CurrFilter_easu(AU2 pos, uvec4 bias[4])
{
    AF3 c;
    FsrEasuF(c, uvec2(pos), bias[0], bias[1], bias[2], bias[3]);
    return vec4(c, 1.0);
}

vec4 CurrFilter_rcas(AU2 pos, uvec4 bias)
{
    AF3 c;
    FsrRcasF(c.r, c.g, c.b, uvec2(pos), bias);

    return vec4(c, 1.0);
}

#include "utils/color.comp"

vec3 aces(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return ((x * (a * x + b)) / (x * (c * x + d) + e));
}

void main()
{

    vec3 color = vec3(0);
    if (ubo.use_fsr == 1)
    {

        uvec4 conf;
        FsrRcasCon(conf, 2);

        FsrEasuCon(rbias[0], rbias[1], rbias[2], rbias[3],
                   float(width) / float(ubo.scale), float(height) / float(ubo.scale),
                   float(width) / float(ubo.scale), float(height) / float(ubo.scale),
                   float(width), float(height));

        // bias[0].x = 0;

        //  vec4 nc = CurrFilter_easu(AU2(fragCoord.x * width , (1 - fragCoord.y) * height ), rbias);
        vec4 rc = CurrFilter_rcas(AU2(fragCoord.x * width, (1 - fragCoord.y) * height), conf);

        color = rc.rgb;
    }
    else
    {
             color = image_value(vec2(fragCoord.x, fragCoord.y)).rgb;
   
   //     color = read_pixel(fragCoord.x, fragCoord.y).normal.rgb;
    }

    outColor = vec4((aces((color))), 1.0f);
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

#define NO_DENOISE
