#include <render/vulkan/img_view.h>

void vulkan_create_imageview(VulkanGfxCtx *ctx, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView)

{
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = (VkComponentMapping){
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = (VkImageSubresourceRange){
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    vk_try$(vkCreateImageView(ctx->device, &viewInfo, NULL, imageView));
}

void vulkan_image_view_init(VulkanGfxCtx *ctx)
{

    int img_view_cnt = ctx->swapchain.images.length;
    vec_init(&ctx->swapchain.img_view);
    vec_resize(&ctx->swapchain.img_view, img_view_cnt);

    for (int i = 0; i < img_view_cnt; i++)
    {
        vulkan_create_imageview(ctx, ctx->swapchain.images.data[i], ctx->swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT, &ctx->swapchain.img_view.data[i]);
    }
}

void vulkan_image_view_deinit(VulkanGfxCtx *ctx)
{
    for (int i = 0; i < ctx->swapchain.img_view.length; i++)
    {
        vkDestroyImageView(ctx->device, ctx->swapchain.img_view.data[i], NULL);
    }
}
