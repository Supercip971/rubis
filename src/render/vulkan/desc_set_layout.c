#include <config.h>
#include <render/vulkan/buffer.h>
#include <render/vulkan/desc_set_layout.h>

void vulkan_desc_create_pool(VulkanCtx *ctx)
{
    VkDescriptorPoolSize pool_0_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
    };
    VkDescriptorPoolSize pool_1_size = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
    };

    VkDescriptorPoolSize pools[] = {pool_0_size, pool_1_size};
    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 2,
        .pPoolSizes = pools,
        .maxSets = 1,
    };

    vk_try$(vkCreateDescriptorPool(ctx->logical_device, &pool_info, NULL, &ctx->descriptor_pool));
}

void vulkan_desc_set_layout(VulkanCtx *ctx)
{
    vulkan_desc_create_pool(ctx);
    /*ù
    VkDescriptorSetLayoutBinding binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };
    */

    VkDescriptorSetLayoutBinding binding_f_0 = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
    };

    VkDescriptorSetLayoutBinding binding_f_1 = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
    };

    VkDescriptorSetLayoutBinding bindings[] = {
        binding_f_0,
        binding_f_1,
    };
    VkDescriptorSetLayoutCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = bindings,
    };

    vk_try$(vkCreateDescriptorSetLayout(ctx->logical_device, &create_info, NULL, &ctx->descriptor_layout));

    ctx->computing_image = vk_buffer_alloc(ctx, 4 * sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 0);
    ctx->config_buf = vk_buffer_alloc(ctx, sizeof(ctx->config_buf), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VulkanConfig *cfg = vk_buffer_map(ctx, ctx->config_buf);

    cfg->width = WINDOW_WIDTH;
    cfg->height = WINDOW_HEIGHT;

    vk_buffer_unmap(ctx, ctx->config_buf);

    VkDescriptorSetAllocateInfo alloc_info =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = ctx->descriptor_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &ctx->descriptor_layout,
        };

    vk_try$(vkAllocateDescriptorSets(ctx->logical_device, &alloc_info, &ctx->descriptor_set));

    VkDescriptorBufferInfo buffer_info = {
        .buffer = ctx->computing_image.buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE,
    };
    VkDescriptorBufferInfo buffer2_info = {
        .buffer = ctx->config_buf.buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE,
    };

    VkWriteDescriptorSet desc_write =
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstArrayElement = 0,
            .dstBinding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .dstSet = ctx->descriptor_set,
            .pBufferInfo = &buffer_info,

        };

    VkWriteDescriptorSet desc_write2 =
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstArrayElement = 0,
            .dstBinding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .dstSet = ctx->descriptor_set,
            .pBufferInfo = &buffer2_info,

        };

    vkUpdateDescriptorSets(ctx->logical_device, 1, &desc_write, 0, NULL);
    vkUpdateDescriptorSets(ctx->logical_device, 1, &desc_write2, 0, NULL);
}

void vulkan_desc_layout_deinit(VulkanCtx *ctx)
{

    vkDestroyDescriptorPool(ctx->logical_device, ctx->descriptor_pool, NULL);
    vk_buffer_free(ctx, ctx->computing_image);
    vkDestroyDescriptorSetLayout(ctx->logical_device, ctx->descriptor_layout, NULL);
}
