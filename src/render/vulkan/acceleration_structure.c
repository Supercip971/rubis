#include <render/vulkan/acceleration_structure.h>
#include <render/vulkan/buffer.h>
#include <render/vulkan/vertex.h>
#include <vulkan/vulkan_core.h>
#include "ds/vec.h"
#include "render/vulkan/command.h"
#include "render/vulkan/vulkan.h"

#ifndef ABANDONNED_CODE
void init_acceleration_structure_mesh(VulkanCtx *self, int id, AccelerationStructures *structures)
{

    Mesh mesh = self->scene.data.meshes.data[id];
    // vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo), const VkBufferDeviceAddressInfo *pInfo)

    VkAccelerationStructureGeometryTrianglesDataKHR _data = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
        .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
        .vertexData.deviceAddress = vk_buffer_addr(self, self->vertex_buffer),
        .vertexStride = sizeof(float) * 4 * SVERTEX_PACKED_COUNT,
        .indexType = VK_INDEX_TYPE_NONE_KHR,
        .indexData = {0},
        .maxVertex = self->scene.data.data.length,
    };

    VkAccelerationStructureGeometryKHR _geometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
        .geometry = {
            .triangles = _data},
        .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
    };

    VkAccelerationStructureBuildRangeInfoKHR offset = {
        .firstVertex = 0,
        .primitiveCount = (mesh.vertices.end - mesh.vertices.start) / (TRIANGLE_PACKED_COUNT),
        .primitiveOffset = mesh.vertices.start * sizeof(float) * 4,
        .transformOffset = 0,
    };

    AccelerationStructure _acceleration_structure = {
        .geometries = _geometry,
        .ranges = offset,
    };
    vec_push(structures, _acceleration_structure);
}

void prepare_blas(VulkanCtx *self, AccelerationStructures *array, VkBuildAccelerationStructureFlagsKHR flags, int id)
{
    VkAccelerationStructureBuildGeometryInfoKHR build_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .flags = flags,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .srcAccelerationStructure = VK_NULL_HANDLE,
        .dstAccelerationStructure = VK_NULL_HANDLE,
        .geometryCount = 1,
        .pGeometries = &array->data[id].geometries,
        .ppGeometries = NULL,
    };

    VkAccelerationStructureBuildSizesInfoKHR size_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
    };

    vec_t(uint32_t) maxprim_count;
    vec_init(&maxprim_count);

    vec_push(&maxprim_count, array->data[id].ranges.primitiveCount);
    // for(int i = 0; i < array.data[i].geometrieslength; i++)
    // {
    //     vec_push(&maxprim_count, array.ranges.data[i].primitiveCount);
    // }

    // from spec, it doesn't read any data,
    // "VkDeviceOrHostAddressKHR members of pBuildInfo are ignored by this command"
    // So I don't expect that we need to offset anything ?
    vkGetAccelerationStructureBuildSizesKHR(self->gfx.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, maxprim_count.data, &size_info);

    array->data[id].build_info = build_info;
    array->data[id].requested_size = size_info.accelerationStructureSize;
    array->data[id].scratch_size = size_info.buildScratchSize;
    array->data[id].flags = build_info.flags;

    vec_deinit(&maxprim_count);
}

typedef vec_t(uint32_t) ObjIndices;

void create_Blas(VulkanCtx *self, AccelerationStructures *array, ObjIndices *indices, VulkanBuffer scratch, VkQueryPool query_pool)
{
    // if (query_pool)
    //{
    //     vkResetQueryPool(self->device, query_pool, 0, indices->length);
    // }

    (void)query_pool;

    // uint32_t query_count = 0;

    for (int i = 0; i < indices->length; i++)
    {

        AccelerationStructure *acceleration_structure = &array->data[indices->data[i]];
        VkAccelerationStructureCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
            .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            .size = acceleration_structure->requested_size,
            .buffer = self->vertex_buffer.buffer,
        };

        acceleration_structure->buffer = vk_buffer_alloc(self, acceleration_structure->requested_size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        create_info.buffer = acceleration_structure->buffer.buffer;
//        VulkanBuffer tlas_buffer = vk_buffer_alloc(self, sizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


        acceleration_structure->build_info.scratchData.deviceAddress = vk_buffer_addr(self, scratch);
        acceleration_structure->build_info.dstAccelerationStructure = acceleration_structure->handle;

        vk_try$(vkCreateAccelerationStructureKHR(self->gfx.device, &create_info, NULL, &acceleration_structure->handle));
    }
}

void create_Tlas(VulkanCtx *self, AccelerationStructures *array, ObjIndices *indices, VkQueryPool query_pool)
{
    // if (query_pool)
    //{
    //     vkResetQueryPool(self->device, query_pool, 0, indices->length);
    // }

    (void)query_pool;

    // uint32_t query_count = 0;

    vec_t(VkAccelerationStructureInstanceKHR) instances;

    vec_init(&instances);


    for (int i = 0; i < indices->length; i++)
    {
        AccelerationStructure *acceleration_structure = &array->data[indices->data[i]];

        VkAccelerationStructureInstanceKHR instance = {
            .transform = {
                .matrix = {
                    {1.0f, 0.0f, 0.0f, 0.0f},
                    {0.0f, 1.0f, 0.0f, 0.0f},
                    {0.0f, 0.0f, 1.0f, 0.0f},
                },
            },
            .instanceCustomIndex = i,
            .mask = 0xFF,
            .instanceShaderBindingTableRecordOffset = 0,
            .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
            .accelerationStructureReference = acceleration_structure->handle_addr,
        };

        vec_push(&instances, instance);
    }

    VulkanBuffer instance_buffer = vk_buffer_alloc(self, instances.length * sizeof(VkAccelerationStructureInstanceKHR), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    volatile void *mem = vk_buffer_map(self, instance_buffer);

    memcpy((void *)mem, instances.data, instances.length * sizeof(VkAccelerationStructureInstanceKHR));

    vk_buffer_unmap(self, instance_buffer);

    VkAccelerationStructureGeometryInstancesDataKHR instances_data = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
        .arrayOfPointers = VK_FALSE,
        .data.deviceAddress = vk_buffer_addr(self, instance_buffer),
    };

    VkAccelerationStructureGeometryKHR tlasGeoInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
        .geometry = {
            .instances = instances_data,
        },
    };

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        .geometryCount = 1,
        .pGeometries = &tlasGeoInfo,
    };

    const uint32_t num_instances = instances.length;

    VkAccelerationStructureBuildSizesInfoKHR sizeInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
    };

    vkGetAccelerationStructureBuildSizesKHR(self->gfx.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &num_instances, &sizeInfo);

    VulkanBuffer tlas_buffer = vk_buffer_alloc(self, sizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkAccelerationStructureCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .size = sizeInfo.accelerationStructureSize,
        .buffer = tlas_buffer.buffer,
    };

    vk_try$(vkCreateAccelerationStructureKHR(self->gfx.device, &create_info, NULL, &self->tlas));

    VulkanBuffer tlas_scratch_buffer = vk_buffer_alloc(self, sizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    buildInfo.dstAccelerationStructure = self->tlas;
    buildInfo.scratchData.deviceAddress = vk_buffer_addr(self, tlas_scratch_buffer);

    VkCommandBuffer tmp_cmd = vk_start_single_time_command(&self->gfx);
    {
        VkAccelerationStructureBuildRangeInfoKHR range = {};
        range.primitiveCount = num_instances;

        const VkAccelerationStructureBuildRangeInfoKHR *ranges[1] = {&range};



        vkCmdBuildAccelerationStructuresKHR(tmp_cmd, 1, &buildInfo, ranges);
    }
    vk_end_single_time_command(&self->gfx, tmp_cmd);



    VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
        .accelerationStructure = self->tlas,
    };
    self->tlas_address = vkGetAccelerationStructureDeviceAddressKHR(self->gfx.device, &addressInfo);
    // int len = array->length;
}

void init_acceleration_structure(VulkanCtx *self)
{

    AccelerationStructures structures = {0};

    vec_init(&structures);

    for (int i = 0; i < self->scene.data.meshes.length; i++)
    {
        init_acceleration_structure_mesh(self, i, &structures);
    }

    size_t max_scratch = 0;

    for (int i = 0; i < self->scene.data.meshes.length; i++)
    {
        prepare_blas(self, &structures, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, i);

        //  if (structures.data[i].flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)
        //  {
        //      number_compact++;
        //  }

        // as_size += structures.data[i].requested_size;
        max_scratch = umax(max_scratch, (size_t)structures.data[i].scratch_size);
    }

    VulkanBuffer scratch_buffer = vk_buffer_alloc(self, max_scratch, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 0);
    // VkBufferDeviceAddressInfo bufferInfo = {
    //     .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
    //     .buffer = scratch_buffer.buffer,
    // };

    // To get the REAL size of the structure instead of a guess
    VkQueryPool query_pool = VK_NULL_HANDLE;

    ObjIndices blas_indices;

    vec_init(&blas_indices);

    for (int i = 0; i < structures.length; i++)
    {

        vec_push(&blas_indices, i);
    }
    VkCommandBuffer cmd = vk_start_single_time_command(&self->gfx);

    create_Blas(self, &structures, &blas_indices, scratch_buffer, query_pool);

    for (int i = 0; i < blas_indices.length; i++)
    {
        AccelerationStructure *acceleration_structure = &structures.data[blas_indices.data[i]];
        const VkAccelerationStructureBuildRangeInfoKHR *ranges[1] = {&acceleration_structure->ranges};

        acceleration_structure->build_info.dstAccelerationStructure = acceleration_structure->handle;
        vkCmdBuildAccelerationStructuresKHR(cmd, 1, &acceleration_structure->build_info, ranges);

        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                             VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                             0, 1, &memoryBarrier, 0, NULL, 0, NULL);

        // for the scratch buffer
        //  vk_try$(vkGetAccelerationStructureBuildSizesKHR(self->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &acceleration_structure->build_info, &acceleration_structure->primitive_count, &acceleration_structure->build_sizes));
    }
    vk_end_single_time_command(&self->gfx, cmd);

    for (int i = 0; i < blas_indices.length; i++)
    {
        AccelerationStructure *acceleration_structure = &structures.data[blas_indices.data[i]];

        VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
            .accelerationStructure = acceleration_structure->handle,
        };
        acceleration_structure->handle_addr = vkGetAccelerationStructureDeviceAddressKHR(self->gfx.device, &addressInfo);
    }

    create_Tlas(self, &structures,& blas_indices, query_pool);
    //    vkDestroyQueryPool(self->device, query_pool, NULL);

 vk_buffer_free(self, scratch_buffer);

 self->accels = structures;
 //abort();
}
void deinit_acceleration_structure(VulkanCtx *self)
{
    for (int i = 0; i < self->accels.length; i++)
    {
        vkDestroyAccelerationStructureKHR(self->gfx.device, self->accels.data[i].handle, NULL);
    }
    vkDestroyAccelerationStructureKHR(self->gfx.device, self->tlas,NULL);
    vec_deinit(&self->accels);
}
#endif
