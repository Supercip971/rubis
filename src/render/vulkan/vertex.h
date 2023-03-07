
#pragma once 

#include <vulkan/vulkan_core.h>
#include <math/vec3.h>

VkVertexInputBindingDescription vulkan_vertex_binding(void);


typedef struct 
{
    VkVertexInputBindingDescription input;
    VkVertexInputAttributeDescription attributes[5];
} VertexDescription;

typedef struct
{
    float x, y;
} TriangleTexPos;

typedef struct 
{
    Vec3 pos; 
    
    TriangleTexPos tc1;
    TriangleTexPos tc2;

    Vec3 normal;
    Vec3 tangent;

} SVertex;

typedef struct
{
    SVertex a;
    SVertex b;
    SVertex c;
} Triangle;

#define SVERTEX_PACKED_COUNT 4
#define TRIANGLE_PACKED_COUNT (SVERTEX_PACKED_COUNT * 3)
SVertex svertex_unpack(const Vec3* packed_data);


Triangle triangle_unpack(const Vec3* packed_data);

void svertex_pack( Vec3* packed_data, SVertex vertex);
void triangle_pack(Vec3* target, Triangle triangle);


VertexDescription vulkan_vertex_desc();
