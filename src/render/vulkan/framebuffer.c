#include <render/vulkan/framebuffer.h>

void vulkan_framebuffer_init(VulkanCtx *ctx)
{
    int fb_count = ctx->swapchain_img_view.length;
    vec_init(&ctx->framebuffers);

    vec_resize(&ctx->framebuffers, fb_count);

    for (int i = 0; i < fb_count; i++)
    {
        VkImageView attachement[] = {
            ctx->swapchain_img_view.data[i],
            ctx->depth_view
        };

        VkFramebufferCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = ctx->render_pass,
            .attachmentCount = 2,
            .pAttachments = attachement,
            .width = ctx->extend.width,
            .height = ctx->extend.height,
            .layers = 1,
        };

        vk_try$(vkCreateFramebuffer(ctx->logical_device, &create_info, NULL, &ctx->framebuffers.data[i]));
    }
}

void vulkan_framebuffer_deinit(VulkanCtx *ctx)
{
    int fb_count = ctx->swapchain_img_view.length;

    for (int i = 0; i < fb_count; i++)
    {
        vkDestroyFramebuffer(ctx->logical_device, ctx->framebuffers.data[i], NULL);
    }
    vec_deinit(&ctx->framebuffers);
}
