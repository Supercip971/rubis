#include <render/vulkan/buffer.h>
#include <render/vulkan/command.h>
#include <render/vulkan/textures.h>
#include <vulkan/vulkan_core.h>
#include "render/vulkan/vulkan.h"


void image_load_from_buffer(VulkanCtx *ctx, VkImage target, uint32_t width, uint32_t height, uint32_t layout_count, VkBuffer buf)
{
    VkCommandBuffer cmd_buf = vk_start_single_time_command(&ctx->gfx);
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

    vk_end_single_time_command(&ctx->gfx, cmd_buf);
}

void swap_image_layout(VulkanCtx *ctx, VkImage image, VkFormat fmt, VkImageLayout old, VkImageLayout new, int layers, bool compute, bool writable)
{
    // TODO: improve this function

    VkCommandBuffer cmd_buf = vk_start_single_time_command(&ctx->gfx);
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

    if (new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (fmt == VK_FORMAT_D32_SFLOAT_S8_UINT || fmt == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    VkPipelineStageFlags src_stage = (old == VK_IMAGE_LAYOUT_UNDEFINED) ? (VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) : (VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkPipelineStageFlags dst_stage = (old == VK_IMAGE_LAYOUT_UNDEFINED) ? (VK_PIPELINE_STAGE_TRANSFER_BIT) : (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    if (old == VK_IMAGE_LAYOUT_UNDEFINED && new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && (  new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (old == VK_IMAGE_LAYOUT_UNDEFINED && new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        printf("[warn] unhandled image layout transition %x -> %x\n", old, new);
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if (compute)
    {
        if (dst_stage == VK_PIPELINE_STAGE_TRANSFER_BIT)
        {
            dst_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            barrier.dstAccessMask = (!writable) ? barrier.dstAccessMask : VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        }
    }
    vkCmdPipelineBarrier(cmd_buf, src_stage, dst_stage, 0, 0, 0, 0, 0, 1, &barrier);

    vk_end_single_time_command(&ctx->gfx, cmd_buf);
}

VkImageView image_view_create(VulkanCtx *ctx, VkImage image, int layers, bool use_float)
{

    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = (layers == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        .format = (!use_float) ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R32G32B32A32_SFLOAT,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = layers,
    };
    VkImageView view;
    vk_try$(vkCreateImageView(ctx->gfx.device, &view_info, NULL, &view));

    if(view == VK_NULL_HANDLE)
    {
        printf("[warn] null handle returned \n");
    }
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
        .maxAnisotropy = 1.0f,

    };

    VkSampler sampler;
    vk_try$(vkCreateSampler(ctx->gfx.device, &sampler_info, NULL, &sampler));

    return sampler;
}

void vulkan_shader_shared_texture_init(VulkanCtx *ctx, VulkanTex *self, int width, int height, bool fragment)
{

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = width,
        .extent.height = height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = (fragment) ? (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) : (VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
    };

    vk_try$(vkCreateImage(ctx->gfx.device, &image_info, NULL, &self->image));

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(ctx->gfx.device, self->image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(ctx, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mem_requirements.size),
    };

    vk_try$(vkAllocateMemory(ctx->gfx.device, &alloc_info, NULL, &self->mem));

    vk_try$(vkBindImageMemory(ctx->gfx.device, self->image, self->mem, 0));

    swap_image_layout(ctx, self->image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, (fragment) ? (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) : (VK_IMAGE_LAYOUT_GENERAL), 1, true, true);

    self->desc_info.imageView = image_view_create(ctx, self->image, 1, true);
    self->desc_info.sampler = image_sampler_create(ctx);
    self->desc_info.imageLayout = (fragment) ? (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) : (VK_IMAGE_LAYOUT_GENERAL);
    self->width = width;
    self->height = height;
}

