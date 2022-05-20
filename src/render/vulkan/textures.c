#include <render/vulkan/buffer.h>
#include <render/vulkan/command.h>
#include <render/vulkan/textures.h>

VulkanBuffer vulkan_scene_texture_data_init(VulkanCtx *ctx, uint32_t *final_sizex, uint32_t *final_sizey)
{
    TexLists texs = ctx->scene.textures;

    size_t tsize = 0;
    size_t maxw = 0;
    size_t maxh = 0;

    for (int i = 0; i < texs.length; i++)
    {
        maxw = fmax(texs.data[i].width, maxw);
        maxh = fmax(texs.data[i].height, maxh);
        printf("candidate[%i]: %u\n  / %u\n ", i, texs.data[i].width, texs.data[i].height);
    }

    tsize = maxw * maxh * sizeof(uint32_t) * texs.length;

    printf("textures sizes: %zu  \n", tsize);

    VulkanBuffer staging_buf = vk_buffer_alloc(ctx, tsize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data = vk_buffer_map(ctx, staging_buf);

    size_t off = 0;
    for (int i = 0; i < texs.length; i++)
    {
        Image c = texs.data[i];
        size_t dst_size = maxw * maxh * sizeof(uint32_t);

        size_t src_size = c.width * c.height * sizeof(uint32_t);
        memcpy(data + off, c.data, src_size);
        off += dst_size;

        image_unload(&c);
    }

    vk_buffer_unmap(ctx, staging_buf);

    *final_sizex = maxw;
    *final_sizey = maxh;
    return staging_buf;
}

void image_load_from_buffer(VulkanCtx *ctx, VkImage target, uint32_t width, uint32_t height, uint32_t layout_count, VkBuffer buf)
{
    VkCommandBuffer cmd_buf = vk_start_single_time_command(ctx);
    VkBufferImageCopy region = {
        .bufferImageHeight = 0,
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = layout_count,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {.width = width, .height = height, .depth = 1},

    };
    vkCmdCopyBufferToImage(cmd_buf, buf, target, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vk_end_single_time_command(ctx, cmd_buf);
}
void swap_image_layout(VulkanCtx *ctx, VkImage image, VkImageLayout old, VkImageLayout new, int layers)
{
    VkCommandBuffer cmd_buf = vk_start_single_time_command(ctx);
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = old,
        .newLayout = new,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = layers,
    };

    if (old == VK_IMAGE_LAYOUT_UNDEFINED)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else
    {

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    VkPipelineStageFlags src_stage = (old == VK_IMAGE_LAYOUT_UNDEFINED) ? (VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) : (VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkPipelineStageFlags dst_stage = (old == VK_IMAGE_LAYOUT_UNDEFINED) ? (VK_PIPELINE_STAGE_TRANSFER_BIT) : (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    vkCmdPipelineBarrier(cmd_buf, src_stage, dst_stage, 0, 0, 0, 0, 0, 1, &barrier);

    vk_end_single_time_command(ctx, cmd_buf);
}

VkImageView image_view_create(VulkanCtx *ctx, VkImage image, int layers)
{

    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = layers,
    };
    VkImageView view;
    vk_try$(vkCreateImageView(ctx->logical_device, &view_info, NULL, &view));

    return view;
}
VkSampler image_sampler_create(VulkanCtx *ctx)
{
    VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f,

    };

    VkSampler sampler;
    vk_try$(vkCreateSampler(ctx->logical_device, &sampler_info, NULL, &sampler));

    return sampler;
}
void vulkan_scene_textures_init(VulkanCtx *ctx)
{
    // FIXME: stop doing fricking dumb thing and use a texture atlas
    // this is only << temporary >>, as I need 1917156118 things to test 1 feature
    // I hope this message get deleted as soon as possible.
    // Now I can say that my raytracer consumes more memory than vscode (not 100% of the time tho).
    uint32_t maxw = 0;
    uint32_t maxh = 0;
    TexLists texs = ctx->scene.textures;

    VulkanBuffer staging_buf = vulkan_scene_texture_data_init(ctx, &maxw, &maxh);
    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = maxw,
        .extent.height = maxh,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = texs.length,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
    };

    vk_try$(vkCreateImage(ctx->logical_device, &image_info, NULL, &ctx->combined_textures_image));

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(ctx->logical_device, ctx->combined_textures_image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(ctx, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mem_requirements.size),
    };

    vk_try$(vkAllocateMemory(ctx->logical_device, &alloc_info, NULL, &ctx->combined_textures_mem));

    vkBindImageMemory(ctx->logical_device, ctx->combined_textures_image, ctx->combined_textures_mem, 0);

    swap_image_layout(ctx, ctx->combined_textures_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texs.length);

    image_load_from_buffer(ctx, ctx->combined_textures_image, maxw, maxh, texs.length, staging_buf.buffer);
    swap_image_layout(ctx, ctx->combined_textures_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texs.length);

    vk_buffer_free(ctx, staging_buf);

    ctx->combined_textures.imageView = image_view_create(ctx, ctx->combined_textures_image, texs.length);
    ctx->combined_textures.sampler = image_sampler_create(ctx);
    ctx->combined_textures.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}
void vulkan_scene_textures_deinit(VulkanCtx *ctx)
{
    (void)ctx;
}
