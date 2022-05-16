#include <render/vulkan/img_view.h>

void vulkan_image_view_init(VulkanCtx *ctx)
{

    int img_view_cnt = ctx->swapchain_images.length;
    vec_init(&ctx->swapchain_img_view);
    vec_resize(&ctx->swapchain_img_view, img_view_cnt);

    for (int i = 0; i < img_view_cnt; i++)
    {
        VkImageViewCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = ctx->swapchain_images.data[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = ctx->swapchain_image_format,

            .components = (VkComponentMapping){
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },

            .subresourceRange = (VkImageSubresourceRange){
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }};

        vk_try$(vkCreateImageView(ctx->logical_device, &create_info, NULL, &ctx->swapchain_img_view.data[i]));
    }
}

void vulkan_image_view_deinit(VulkanCtx *ctx)
{
    for (int i = 0; i < ctx->swapchain_img_view.length; i++)
    {
        vkDestroyImageView(ctx->logical_device, ctx->swapchain_img_view.data[i], NULL);
    }
}
