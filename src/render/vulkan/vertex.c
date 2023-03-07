#include "vertex.h"
#include "obj/mesh.h"

size_t svertex_packed_count()
{
    return SVERTEX_PACKED_COUNT;
}

void svertex_pack( Vec3* packed_data, SVertex vertex)
{
    packed_data[0] = vertex.pos;
    packed_data[1] = vertex.normal;
    packed_data[2] = vertex.tangent;
    packed_data[3].x = vertex.tc1.x;
    packed_data[3].y = vertex.tc1.y;
    packed_data[3].z = vertex.tc2.x;
    packed_data[3]._padding = vertex.tc2.y;
}

SVertex svertex_unpack(const Vec3* packed_data)
{
    SVertex vertex = {};
    vertex.pos = packed_data[0];
    vertex.normal = packed_data[1];
    vertex.tangent = packed_data[2];
    vertex.tc1.x = packed_data[3].x;
    vertex.tc1.y = packed_data[3].y;
    vertex.tc2.x = packed_data[3].z;
    vertex.tc2.y = packed_data[3]._padding;
    return vertex;
}

void triangle_pack(Vec3 *target, Triangle triangle)
{
    svertex_pack(target, triangle.a);
    svertex_pack(target + svertex_packed_count() * 1, triangle.b);
    svertex_pack(target + svertex_packed_count() * 2, triangle.c);
}

Triangle triangle_unpack(const Vec3 *packed_data)
{
    Triangle triangle = {};
    triangle.a = svertex_unpack(packed_data);
    triangle.b = svertex_unpack(packed_data + svertex_packed_count() * 1);
    triangle.c = svertex_unpack(packed_data + svertex_packed_count() * 2);
    return triangle;
}



/*
VkVertexInputBindingDescription vulkan_vertex_binding(void)
{
    return (VkVertexInputBindingDescription){
        .binding = 0,
        .stride = sizeof(float) * 4 * MESH_VERTICE_COUNT,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}


VertexDescription vulkan_vertex_desc()
{
    VertexDescription desc = {
        .input  = vulkan_vertex_binding(),
        .attributes = {
            [0] = (VkVertexInputAttributeDescription){
                .binding = 0,
                .location = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = 0,
            }
        },
    };

}*/
