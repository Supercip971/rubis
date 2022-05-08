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

    VkDescriptorPoolSize pool_2_size = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
    };

    VkDescriptorPoolSize pool_3_size = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
    };

    VkDescriptorPoolSize pools[] = {pool_0_size, pool_1_size, pool_2_size, pool_3_size};
    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 4,
        .pPoolSizes = pools,
        .maxSets = 1,
    };

    vk_try$(vkCreateDescriptorPool(ctx->logical_device, &pool_info, NULL, &ctx->descriptor_pool));
}

static void scene_buf_init(VulkanCtx* ctx)
{
    size_t mesh_buf_size = sizeof(Mesh) * ctx->scene.meshes.length;
    size_t mesh_data_buf_size = sizeof(Vec3) * ctx->scene.data.length;

    printf("init-scene: %zu %zu\n", mesh_buf_size, mesh_data_buf_size);

    ctx->mesh_buf = vk_buffer_alloc(ctx, mesh_buf_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);
    ctx->mesh_data_buf = vk_buffer_alloc(ctx, mesh_data_buf_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);

    printf("AAAA\n");

}
 void scene_buf_value_init(VulkanCtx* ctx)
{
    size_t mesh_buf_size = sizeof(Mesh) * ctx->scene.meshes.length;
    size_t mesh_data_buf_size = sizeof(Vec3) * ctx->scene.data.length;


    VulkanBuffer temp_buf = vk_buffer_alloc(ctx, fmax(mesh_buf_size, mesh_data_buf_size), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    volatile void *dat = vk_buffer_map(ctx, temp_buf);


    memcpy((void*)dat, ctx->scene.meshes.data, mesh_buf_size);

    vk_buffer_copy(ctx, ctx->mesh_buf, temp_buf);

    memcpy((void*)dat, ctx->scene.data.data, mesh_data_buf_size);

    vk_buffer_copy(ctx, ctx->mesh_data_buf, temp_buf);




    vk_buffer_unmap(ctx,temp_buf);


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

    VkDescriptorSetLayoutBinding binding_f_2 = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags =  VK_SHADER_STAGE_COMPUTE_BIT,
    };

    VkDescriptorSetLayoutBinding binding_f_3 = {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    };
    VkDescriptorSetLayoutBinding bindings[] = {
        binding_f_0,
        binding_f_1,
        binding_f_2,
        binding_f_3
    };
    VkDescriptorSetLayoutCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 4,
        .pBindings = bindings,
    };

    vk_try$(vkCreateDescriptorSetLayout(ctx->logical_device, &create_info, NULL, &ctx->descriptor_layout));

    ctx->computing_image = vk_buffer_alloc(ctx, 4 * sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 0);
    ctx->config_buf = vk_buffer_alloc(ctx, sizeof(ctx->config_buf) * 2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    scene_buf_init(ctx);


   volatile VulkanConfig *cfg = vk_buffer_map(ctx, ctx->config_buf);
    *cfg = (VulkanConfig){};
    cfg->width = WINDOW_WIDTH;
    cfg->height = WINDOW_HEIGHT;
    cfg->cam_up = vec3_create(0, 1, 0);

    cfg->cam_pos = vec3_create(0, 0, -2);

    cfg->cam_look = vec3_create(0, 0, 0);
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
    VkDescriptorBufferInfo buffer3_info = {
        .buffer = ctx->mesh_data_buf.buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE,
    };
    VkDescriptorBufferInfo buffer4_info = {
        .buffer = ctx->mesh_buf.buffer,
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

    VkWriteDescriptorSet desc_write3 =
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstArrayElement = 0,
            .dstBinding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .dstSet = ctx->descriptor_set,
            .pBufferInfo = &buffer3_info,

        };
    VkWriteDescriptorSet desc_write4 =
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstArrayElement = 0,
            .dstBinding = 3,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .dstSet = ctx->descriptor_set,
            .pBufferInfo = &buffer4_info,

        };
    vkUpdateDescriptorSets(ctx->logical_device, 1, &desc_write, 0, NULL);
    vkUpdateDescriptorSets(ctx->logical_device, 1, &desc_write2, 0, NULL);

    vkUpdateDescriptorSets(ctx->logical_device, 1, &desc_write3, 0, NULL);

    vkUpdateDescriptorSets(ctx->logical_device, 1, &desc_write4, 0, NULL);
}

void vulkan_desc_layout_deinit(VulkanCtx *ctx)
{

    vkDestroyDescriptorPool(ctx->logical_device, ctx->descriptor_pool, NULL);
    vk_buffer_free(ctx, ctx->computing_image);
    vkDestroyDescriptorSetLayout(ctx->logical_device, ctx->descriptor_layout, NULL);
}
