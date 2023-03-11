#include <config.h>
#include <render/vulkan/buffer.h>
#include <render/vulkan/desc_set_layout.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    VulkanBuffer *target;
    VkShaderStageFlags flag;
    VkDescriptorType type;
    VkDescriptorImageInfo image;
} ShaderDescriptor;

typedef vec_t(ShaderDescriptor) ShaderDescriptors;

static void scene_buf_init(VulkanCtx *ctx)
{
    size_t mesh_buf_size = sizeof(Mesh) * ctx->scene.meshes.length;
    size_t mesh_data_buf_size = sizeof(Vec3) * ctx->scene.data.length;
    size_t mesh_bvh_size = sizeof(BvhEntry) * ctx->bvh_data.length;

    VkBufferUsageFlags compute_buffer_usage_flag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    ctx->mesh_buf = vk_buffer_alloc(ctx, mesh_buf_size, compute_buffer_usage_flag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ctx->mesh_data_buf = vk_buffer_alloc(ctx, mesh_data_buf_size, compute_buffer_usage_flag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ctx->bvh_buf = vk_buffer_alloc(ctx, mesh_bvh_size, compute_buffer_usage_flag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

static void vulkan_descriptor_buffer_init(VulkanCtx *ctx)
{
    ctx->computing_image = vk_buffer_alloc(ctx, 4 * sizeof(float) * ctx->aligned_width * ctx->aligned_height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   // ctx->fragment_image = vk_buffer_alloc(ctx, 4 * sizeof(float) * ctx->aligned_width * ctx->aligned_height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ctx->config_buf = vk_buffer_alloc(ctx, sizeof(*ctx->cfg), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    scene_buf_init(ctx);
}

void shader_descriptors_init(VulkanCtx *ctx, ShaderDescriptors *desc)
{
    vec_init(desc);


    vec_push(desc, ((ShaderDescriptor){
                       .image = ctx->comp_targ.desc_info,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                   }));

    vec_push(desc, ((ShaderDescriptor){
                       .target = &ctx->config_buf,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                       .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                   }));

    vec_push(desc, ((ShaderDescriptor){
                       .target = &ctx->mesh_data_buf,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                   }));

    vec_push(desc, ((ShaderDescriptor){
                       .target = &ctx->mesh_buf,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                   }));

    //    vec_push(desc, ((ShaderDescriptor){
    //                       .image = ctx->frag_targ.desc_info,
    //                       .flag = VK_SHADER_STAGE_FRAGMENT_BIT ,
    //                       .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    //                   }));
    //

    vec_push(desc, ((ShaderDescriptor){
                       .image = ctx->fragment_image.desc_info,
                       .flag = VK_SHADER_STAGE_FRAGMENT_BIT,
                       .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                   }));
    vec_push(desc, ((ShaderDescriptor){
                       .target = &ctx->bvh_buf,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                   }));

    vec_push(desc, ((ShaderDescriptor){
                       .image = ctx->combined_textures.desc_info,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                   }));

    vec_push(desc, ((ShaderDescriptor){
                       .image = ctx->skymap.desc_info,
                       .flag = VK_SHADER_STAGE_COMPUTE_BIT,
                       .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                   }));
}

static void vulkan_desc_pool_init(VulkanCtx *ctx, ShaderDescriptors descriptors)
{
    vec_t(VkDescriptorPoolSize) pool_sizes;
    vec_init(&pool_sizes);

    for (int i = 0; i < descriptors.length; i++)
    {
        ShaderDescriptor current = descriptors.data[i];

        vec_push(&pool_sizes, ((VkDescriptorPoolSize){
                                  .descriptorCount = 1,
                                  .type = current.type,
                              }));
    }

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = pool_sizes.length,
        .pPoolSizes = pool_sizes.data,
        .maxSets = 1,
    };

    vk_try$(vkCreateDescriptorPool(ctx->logical_device, &pool_info, NULL, &ctx->descriptor_pool));

    vec_deinit(&pool_sizes);
}

static void vulkan_desc_layout_init(VulkanCtx *ctx, ShaderDescriptors descriptors)
{
    vec_t(VkDescriptorSetLayoutBinding) bindings;
    vec_init(&bindings);

    for (int i = 0; i < descriptors.length; i++)
    {
        ShaderDescriptor current = descriptors.data[i];

        VkDescriptorSetLayoutBinding binding = {
            .binding = i,
            .descriptorType = current.type,
            .stageFlags = current.flag,
            .descriptorCount = 1,
        };

        vec_push(&bindings, binding);
    }

    VkDescriptorSetLayoutCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindings.length,
        .pBindings = bindings.data,
    };

    vk_try$(vkCreateDescriptorSetLayout(ctx->logical_device, &create_info, NULL, &ctx->descriptor_layout));

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = ctx->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &ctx->descriptor_layout,
    };

    vk_try$(vkAllocateDescriptorSets(ctx->logical_device, &alloc_info, &ctx->descriptor_set));
}

static void vulkan_desc_layout_update(VulkanCtx *ctx, ShaderDescriptors descriptors)
{
    for (int i = 0; i < descriptors.length; i++)
    {

        ShaderDescriptor current = descriptors.data[i];

        if (current.type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || current.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || current.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        {
            VkDescriptorImageInfo buffer_info = current.image;

            VkWriteDescriptorSet desc_write = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstArrayElement = 0,
                .dstBinding = i,
                .descriptorType = current.type,
                .descriptorCount = 1,
                .dstSet = ctx->descriptor_set,
                .pImageInfo = &buffer_info,
            };

            vkUpdateDescriptorSets(ctx->logical_device, 1, &desc_write, 0, NULL);
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
            .descriptorCount = 1,
            .dstSet = ctx->descriptor_set,
            .pBufferInfo = &buffer_info,
        };

        vkUpdateDescriptorSets(ctx->logical_device, 1, &desc_write, 0, NULL);
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

    vkDestroyDescriptorPool(ctx->logical_device, ctx->descriptor_pool, NULL);
    vk_buffer_free(ctx, ctx->computing_image);

   // vk_buffer_free(ctx, ctx->fragment_image);

    vk_buffer_free(ctx, ctx->bvh_buf);

    vk_buffer_free(ctx, ctx->mesh_buf);

    vk_buffer_free(ctx, ctx->mesh_data_buf);
    vkDestroyDescriptorSetLayout(ctx->logical_device, ctx->descriptor_layout, NULL);
}

void scene_buf_value_init(VulkanCtx *ctx)
{
    size_t mesh_buf_size = sizeof(Mesh) * ctx->scene.meshes.length;
    size_t mesh_data_buf_size = sizeof(Vec3) * ctx->scene.data.length;
    size_t mesh_bvh_size = sizeof(BvhEntry) * ctx->bvh_data.length;

    VulkanBuffer temp_buf = vk_buffer_alloc(ctx,
                                            fmax(fmax(mesh_buf_size, mesh_data_buf_size), mesh_bvh_size),
                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    volatile void *dat = vk_buffer_map(ctx, temp_buf);

    // mesh buffer
    memcpy((void *)dat, ctx->scene.meshes.data, mesh_buf_size);
    vk_buffer_copy(ctx, ctx->mesh_buf, temp_buf);

    // mesh data buffer
    memcpy((void *)dat, ctx->scene.data.data, mesh_data_buf_size);
    vk_buffer_copy(ctx, ctx->mesh_data_buf, temp_buf);

    // bvh buffer
    memcpy((void *)dat, ctx->bvh_data.data, mesh_bvh_size);
    vk_buffer_copy(ctx, ctx->bvh_buf, temp_buf);

    vk_buffer_unmap(ctx, temp_buf);
    vk_buffer_set(ctx, ctx->computing_image, (uint32_t)(0.0f));

    volatile VulkanConfig *cfg = vk_buffer_map(ctx, ctx->config_buf);
    *cfg = (VulkanConfig){};
    cfg->width = ctx->aligned_width;
    cfg->height = ctx->aligned_height;
    cfg->cam_up = vec3_create(0, 1, 0);

    cfg->cam_pos = vec3_create(0, 0, -2);

    cfg->cam_look = vec3_create(0, 0, 0);
    vk_buffer_unmap(ctx, ctx->config_buf);
}
