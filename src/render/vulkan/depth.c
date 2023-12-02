
#include <render/vulkan/depth.h>
#include <vulkan/vulkan_core.h>
#include "render/vulkan/buffer.h"
#include <render/vulkan/textures.h>


typedef vec_t(VkFormat) VulkanFormats;

static VkFormat find_depth_format(VulkanCoreCtx* ctx, VulkanFormats* candidates, VkImageTiling tiling, VkFormatFeatureFlags feats)
{
    for (int i = 0; i < candidates->length; i++)
    {
        VkFormat format = candidates->data[i];
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(ctx->physical_device, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & feats) == feats)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & feats) == feats)
        {
            return format;
        }
    }
    printf("unable to find a suitable depth format\n");
    abort();
    return VK_FORMAT_UNDEFINED;
}

void vulkan_depth_target_init(VulkanCtx* self)
{

    VulkanFormats candidates = {0};
    vec_init(&candidates);
    vec_push(&candidates, VK_FORMAT_D32_SFLOAT);
    vec_push(&candidates, VK_FORMAT_D32_SFLOAT_S8_UINT);
    vec_push(&candidates, VK_FORMAT_D24_UNORM_S8_UINT);

    VulkanTex tex = {0};

    VkFormat depth_format = find_depth_format(&self->core, &candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    tex.fmt = depth_format;
    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = self->gfx.swapchain.extend.width,
        .extent.height = self->gfx.swapchain.extend.height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = depth_format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,

    };

    tex.width = self->gfx.swapchain.extend.width;
    tex.height = self->gfx.swapchain.extend.height;
    vk_try$(vkCreateImage(self->gfx.device, &image_info, NULL, &tex.image));

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(self->gfx.device, tex.image, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = find_memory_type(self, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mem_reqs.size),
    };

    vk_try$(vkAllocateMemory(self->gfx.device, &alloc_info, NULL, &tex.mem));

    vk_try$(vkBindImageMemory(self->gfx.device, tex.image, tex.mem, 0));


    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = tex.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depth_format,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };
    VkImageView view;
    vk_try$(vkCreateImageView(self->gfx.device, &view_info, NULL, &view));

    swap_image_layout(self, tex.image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1,0,0);

    self->gfx.depth_view = view;
    self->gfx.depth_buffer = tex;
        //VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    vec_deinit(&candidates);
}
void vulkan_depth_target_deinit(VulkanCtx* self)
{
    vkDestroyImageView(self->gfx.device, self->gfx.depth_view, NULL);

    vkFreeMemory(self->gfx.device, self->gfx.depth_buffer.mem, NULL);

    vkDestroyImage(self->gfx.device, self->gfx.depth_buffer.image, NULL);
}
