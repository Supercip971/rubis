/*#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;


void main() {
    outColor = vec4(fragColor, 1.0);
}*/


#version 460 core
/*#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;


void main() {
    outColor = vec4(fragColor, 1.0);
}*/

#extension GL_ARB_separate_shader_objects : enable
#define A_GPU 1
#define A_GLSL 1

struct Pixel
{
    vec4 value;
};

layout(binding = 3) uniform sampler2D buf;

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


uint width = uint(ubo.width);
uint height = uint(ubo.height);

//layout(location = 0) in vec2 fragCoord;
layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;
 in vec4 gl_FragCoord;
vec2 image_uv(vec2 uv)
{

    float u = uv.x / ubo.scale;
    float v = ((1.0 - uv.y) / ubo.scale);
    return (vec2(u, v));
}

#define INV_SQRT_OF_2PI 0.39894228040143267793994605993439  // 1.0/SQRT_OF_2PI
#define INV_PI 0.31830988618379067153776752674503

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Copyright (c) 2018-2019 Michele Morrone
//  All rights reserved.
//
//  https://michelemorrone.eu - https://BrutPitt.com
//
//  me@michelemorrone.eu - brutpitt@gmail.com
//  twitter: @BrutPitt - github: BrutPitt
//  
//  https://github.com/BrutPitt/glslSmartDeNoise/
//
//  This software is distributed under the terms of the BSD 2-Clause license
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//  smartDeNoise - parameters
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  sampler2D tex     - sampler image / texture
//  vec2 uv           - actual fragment coord
//  float sigma  >  0 - sigma Standard Deviation
//  float kSigma >= 0 - sigma coefficient 
//      kSigma * sigma  -->  radius of the circular kernel
//  float threshold   - edge sharpening threshold 

vec4 smartDeNoise(sampler2D tex, vec2 uv, float sigma, float kSigma, float threshold)
{
    float radius = round(kSigma*sigma);
    float radQ = radius * radius;

    float invSigmaQx2 = .5 / (sigma * sigma);      // 1.0 / (sigma^2 * 2.0)
    float invSigmaQx2PI = INV_PI * invSigmaQx2;    // 1/(2 * PI * sigma^2)

    float invThresholdSqx2 = .5 / (threshold * threshold);     // 1.0 / (sigma^2 * 2.0)
    float invThresholdSqrt2PI = INV_SQRT_OF_2PI / threshold;   // 1.0 / (sqrt(2*PI) * sigma^2)

    vec4 centrPx = texture(tex,uv); 

    float zBuff = 0.0;
    vec4 aBuff = vec4(0.0);
    vec2 size = vec2(textureSize(tex, 0));

    vec2 d;
    for (d.x=-radius; d.x <= radius; d.x++) {
        float pt = sqrt(radQ-d.x*d.x);       // pt = yRadius: have circular trend
        for (d.y=-pt; d.y <= pt; d.y++) {
            float blurFactor = exp( -dot(d , d) * invSigmaQx2 ) * invSigmaQx2PI;

            vec4 walkPx =  texture(tex,uv+d/size);
            vec4 dC = walkPx-centrPx;
            float deltaFactor = exp( -dot(dC, dC) * invThresholdSqx2) * invThresholdSqrt2PI * blurFactor;

            zBuff += deltaFactor;
            aBuff += deltaFactor*walkPx;
        }
    }
    return aBuff/zBuff;
}
vec4 image_sample(vec2 uv)
{
    return smartDeNoise(buf, uv, 9, 3*9, 0.135); 
}


vec4 image_value(vec2 uv)
{
    return (texture(buf, image_uv(uv)));
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
    return CurrFilter_easu(AU2(p.x ,p.y ), rbias);
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
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}


void main()
{
    vec2 fragCoord = gl_FragCoord.xy / vec2(width, height);
    vec3 color = vec3(0);
    if (ubo.use_fsr == 1)
    {

        uvec4 conf; 
        FsrRcasCon(conf, 2);


        FsrEasuCon(rbias[0], rbias[1], rbias[2], rbias[3],
                   float(width) / float(ubo.scale), float(height) / float(ubo.scale),
                   float(width) / float(ubo.scale), float(height) / float(ubo.scale),
                   float(width), float(height));

        //bias[0].x = 0;

      //  vec4 nc = CurrFilter_easu(AU2(fragCoord.x * width , (1 - fragCoord.y) * height ), rbias);
        vec4 rc = CurrFilter_rcas(AU2(fragCoord.x * width, (1 - fragCoord.y) * height), conf);

        color = rc.rgb;
      
    }
    else
    {
        color = image_value(fragCoord.xy).rgb;
   //     color = image_value(fragCoord.xy).rgb;
    }

    outColor = vec4(linear_to_srgb(aces((color +fragColor )/2)), 1.0f);
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

//
//void main() {
//    outColor = vec4(fragColor, 1.0);
//}