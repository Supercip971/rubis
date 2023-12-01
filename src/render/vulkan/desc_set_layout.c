#include <config.h>
#include <render/vulkan/buffer.h>
#include <render/vulkan/desc_set_layout.h>
#include <vulkan/vulkan_core.h>
#include "obj/scene.h"
#include "render/vulkan/vulkan.h"

typedef struct
{
    VulkanBuffer *target;
    VkShaderStageFlags flag;
    VkDescriptorType type;
    VkDescriptorImageInfo *image;
    int count;
    VkDescriptorBindingFlagsEXT binding_flags;
    VkAccelerationStructureKHR* accel_structure;
} ShaderDescriptor;

typedef vec_t(ShaderDescriptor) ShaderDescriptors;

static void scene_buf_init(VulkanCtx *ctx)
{
    size_t mesh_buf_size = umax(16, sizeof(Mesh) * ctx->scene.meshes.length);
    size_t mesh_emit_size = umax(16, sizeof(EmissiveIndex) * ctx->scene.mesh_emissive_indices.length);

    size_t mesh_data_buf_size = umax(16, sizeof(Vec3) * ctx->scene.data.length);
    //size_t mesh_bvh_size = umax(16, sizeof(BvhEntry) * ctx->bvh_data.length);


    VkBufferUsageFlags compute_buffer_usage_flag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT ;

    ctx->mesh_buf = vk_buffer_alloc(ctx, mesh_buf_size, compute_buffer_usage_flag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ctx->emissive_buf = vk_buffer_alloc(ctx, mesh_emit_size, compute_buffer_usage_flag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


    ctx->mesh_data_buf = vk_buffer_alloc(ctx, mesh_data_buf_size, compute_buffer_usage_flag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
 //   ctx->bvh_buf = vk_buffer_alloc(ctx, mesh_bvh_size, compute_buffer_usage_flag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

static void vulkan_descriptor_buffer_init(VulkanCtx *ctx)
{

    size_t info_buf_size = umax(16, sizeof(PixelInfo) * ctx->core.aligned_width * ctx->core.aligned_height);
    ctx->result_info_buf = vk_buffer_alloc(ctx, info_buf_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ctx->computing_image = vk_buffer_alloc(ctx, 4 * sizeof(float) * ctx->core.aligned_width * ctx->core.aligned_height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    // ctx->fragment_image = vk_buffer_alloc(ctx, 4 * sizeof(float) * ctx->aligned_width * ctx->aligned_height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    // ctx->config_buf = vk_buffer_alloc(ctx, sizeof(*ctx->cfg), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    scene_buf_init(ctx);
}

void shader_descriptors_init(VulkanCtx *ctx, ShaderDescriptors *desc)
{
    vec_init(desc);
    // 0
    vec_push(desc, ((ShaderDescriptor){
                       .image = &ctx->gfx.comp_targ.desc_info,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                       .count = 1,
                   }));

    //  vec_push(desc, ((ShaderDescriptor){
    //                     .target = &ctx->config_buf,
    //                     .flag = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
    //                     .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    //                     .count = 1,
    //                 }));
    // 1
    vec_push(desc, ((ShaderDescriptor){
                       .target = &ctx->mesh_data_buf,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                       .count = 1,
                   }));
    // 2
    vec_push(desc, ((ShaderDescriptor){
                       .target = &ctx->mesh_buf,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                       .count = 1,
                   }));

    //    vec_push(desc, ((ShaderDescriptor){
    //                       .image = ctx->frag_targ.desc_info,
    //                       .flag = VK_SHADER_STAGE_FRAGMENT_BIT ,
    //                       .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    //                   }));
    //
    // 3
    vec_push(desc, ((ShaderDescriptor){
                       .image = &ctx->gfx.fragment_image.desc_info,
                       .flag = VK_SHADER_STAGE_FRAGMENT_BIT,
                       .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,
                       .count = 1,
                   }));
    // 4
    vec_push(desc, ((ShaderDescriptor){
                       .accel_structure = &ctx->tlas,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                       .count = 1,
                   }));
   // vec_push(desc, ((ShaderDescriptor){
   //                    .target = &ctx->bvh_buf,
   //                    .flag = VK_SHADER_STAGE_COMPUTE_BIT,
   //                    .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
   //                    .count = 1,
   //                }));
    // 5
    vec_push(desc, ((ShaderDescriptor){
                       .image = ctx->combined_textures.final_info.data,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                       .count = ctx->combined_textures.final_info.length,
                       .binding_flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
                       }));
    // 6
    vec_push(desc, ((ShaderDescriptor){
                       .image = &ctx->skymap.desc_info,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       .count = 1,
                   }));
    // 7
    vec_push(desc, ((ShaderDescriptor){
                       .target = &ctx->emissive_buf,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                       .count = 1,
                   }));
    // 8
    vec_push(desc, ((ShaderDescriptor){
                       .image = &ctx->scene_sampler.desc_info,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_SAMPLER,
                       .count = 1,
                   }));

    // 9
    vec_push(desc, ((ShaderDescriptor){
                       .target= &ctx->result_info_buf,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                       .count = 1,
                   }));

    // 10

}

static void vulkan_desc_pool_init(VulkanCtx *ctx, ShaderDescriptors descriptors)
{
    vec_t(VkDescriptorPoolSize) pool_sizes;
    vec_init(&pool_sizes);

    for (int i = 0; i < descriptors.length; i++)
    {
        ShaderDescriptor current = descriptors.data[i];

        vec_push(&pool_sizes, ((VkDescriptorPoolSize){
                                  .descriptorCount = current.count,
                                  .type = current.type,
                              }));
    }

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = pool_sizes.length,
        .pPoolSizes = pool_sizes.data,
        .maxSets = 1,
    };

    vk_try$(vkCreateDescriptorPool(ctx->gfx.device, &pool_info, NULL, &ctx->descriptor_pool));

    vec_deinit(&pool_sizes);
}

static void vulkan_desc_layout_init(VulkanCtx *ctx, ShaderDescriptors descriptors)
{
    vec_t(VkDescriptorSetLayoutBinding) bindings;
    vec_t(VkDescriptorBindingFlags) binding_flags;
    vec_init(&bindings);
    vec_init(&binding_flags);

    for (int i = 0; i < descriptors.length; i++)
    {
        ShaderDescriptor current = descriptors.data[i];

        VkDescriptorSetLayoutBinding binding = {
            .binding = i,
            .descriptorType = current.type,
            .stageFlags = current.flag,
            .descriptorCount = current.count,
            .pImmutableSamplers = NULL,
        };

        vec_push(&bindings, binding);
        vec_push(&binding_flags, current.binding_flags);
    }

    VkDescriptorSetLayoutCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindings.length,
        .pBindings = bindings.data,
        .flags = 0,
    };

    vk_try$(vkCreateDescriptorSetLayout(ctx->gfx.device, &create_info, NULL, &ctx->gfx.descriptor_layout));

    //VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info = {
    //    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
    //    .pNext = NULL,
    //    .bindingCount = bindings.length,
    //    .pBindingFlags = binding_flags.data,
    //};
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = ctx->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &ctx->gfx.descriptor_layout,
        .pNext = NULL,
    };

    //uint32_t max_count = ctx->combined_textures.textures.length;
    //VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_info = {
    //    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
    //    .descriptorSetCount = 1,
    //    .pDescriptorCounts = &max_count,
    //};
    //alloc_info.pNext = &count_info;
    vk_try$(vkAllocateDescriptorSets(ctx->gfx.device, &alloc_info, &ctx->gfx.descriptor_set));
}

static void vulkan_desc_layout_update(VulkanCtx *ctx, ShaderDescriptors descriptors)
{
    for (int i = 0; i < descriptors.length; i++)
    {

        ShaderDescriptor current = descriptors.data[i];

        if (current.type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
            current.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
            current.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
            current.type == VK_DESCRIPTOR_TYPE_SAMPLER)
        {

            VkWriteDescriptorSet desc_write = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstArrayElement = 0,
                .dstBinding = i,
                .descriptorType = current.type,
                .descriptorCount = current.count,
                .dstSet = ctx->gfx.descriptor_set,
                .pImageInfo = current.image,
            };

            vkUpdateDescriptorSets(ctx->gfx.device, 1, &desc_write, 0, NULL);
            continue;
        }

        else if(current.type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
        {
            VkWriteDescriptorSetAccelerationStructureKHR desc_acceleration = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
                .accelerationStructureCount = 1,
                .pAccelerationStructures = current.accel_structure,
            };

            VkWriteDescriptorSet desc_write = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstArrayElement = 0,
                .dstBinding = i,
                .descriptorType = current.type,
                .descriptorCount = current.count,
                .dstSet = ctx->gfx.descriptor_set,
                .pNext = &desc_acceleration,
            };

            vkUpdateDescriptorSets(ctx->gfx.device, 1, &desc_write, 0, NULL);
            continue;
        }
        VkDescriptorBufferInfo buffer_info = {
            .buffer = current.target->buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE,
        };

        VkWriteDescriptorSet desc_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstArrayElement = 0,
            .dstBinding = i,
            .descriptorType = current.type,
            .descriptorCount = current.count,
            .dstSet = ctx->gfx.descriptor_set,
            .pBufferInfo = &buffer_info,
        };

        vkUpdateDescriptorSets(ctx->gfx.device, 1, &desc_write, 0, NULL);
    }
}

void vulkan_desc_set_layout(VulkanCtx *ctx)
{
    vulkan_descriptor_buffer_init(ctx);

    ShaderDescriptors descriptors;
    vec_init(&descriptors);
    shader_descriptors_init(ctx, &descriptors);

    vulkan_desc_pool_init(ctx, descriptors);

    vulkan_desc_layout_init(ctx, descriptors);

    vulkan_desc_layout_update(ctx, descriptors);
}

void vulkan_desc_layout_deinit(VulkanCtx *ctx)
{

    vkDestroyDescriptorPool(ctx->gfx.device, ctx->descriptor_pool, NULL);
    vk_buffer_free(ctx, ctx->computing_image);

    // vk_buffer_free(ctx, ctx->fragment_image);

    //vk_buffer_free(ctx, ctx->bvh_buf);

    vk_buffer_free(ctx, ctx->mesh_buf);

    vk_buffer_free(ctx, ctx->mesh_data_buf);
    vkDestroyDescriptorSetLayout(ctx->gfx.device, ctx->gfx.descriptor_layout, NULL);
}

void scene_buf_value_init(VulkanCtx *ctx)
{
    size_t mesh_buf_size = sizeof(Mesh) * ctx->scene.meshes.length;
    size_t mesh_data_buf_size = sizeof(Vec3) * ctx->scene.data.length;
    size_t mesh_bvh_size = 0;
    size_t mesh_idx_size = sizeof(EmissiveIndex) * ctx->scene.mesh_emissive_indices.length;


    VulkanBuffer temp_buf = vk_buffer_alloc(ctx,
                                            fmax(fmax(fmax(mesh_buf_size, mesh_data_buf_size), mesh_bvh_size), mesh_idx_size),
                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    volatile void *dat = vk_buffer_map(ctx, temp_buf);

    // mesh buffer
    memcpy((void *)dat, ctx->scene.meshes.data, mesh_buf_size);
    vk_buffer_copy(ctx, ctx->mesh_buf, temp_buf);

    // emmissive data buffer
    memcpy((void *)dat, ctx->scene.mesh_emissive_indices.data, mesh_idx_size);
    vk_buffer_copy(ctx, ctx->emissive_buf, temp_buf);

    // mesh data buffer
    memcpy((void *)dat, ctx->scene.data.data, mesh_data_buf_size);
    vk_buffer_copy(ctx, ctx->mesh_data_buf, temp_buf);

    // bvh buffer
   // memcpy((void *)dat, ctx->bvh_data.data, mesh_bvh_size);
   // vk_buffer_copy(ctx, ctx->bvh_buf, temp_buf);

    vk_buffer_unmap(ctx, temp_buf);
    vk_buffer_set(ctx, ctx->computing_image, (uint32_t)(0.0f));

    ctx->cfg.width = ctx->core.aligned_width;
    ctx->cfg.height = ctx->core.aligned_height;
    ctx->cfg.cam_up = vec3_create(0, 1, 0);

    ctx->cfg.cam_pos = vec3_create(0, 0, -2);

    ctx->cfg.cam_look = vec3_create(0, 0, 0);
}
