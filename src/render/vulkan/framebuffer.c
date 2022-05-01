#include <render/vulkan/framebuffer.h>

void vulkan_framebuffer_init(VulkanCtx *ctx)
{

    int fb_count = ctx->swapchain_img_view.length;
    vec_init(&ctx->framebuffers);

    vec_reserve(&ctx->framebuffers, fb_count);
    ctx->framebuffers.length = fb_count;

    for (int i = 0; i < fb_count; i++)
    {
        VkImageView attachement[] = {
            ctx->swapchain_img_view.data[i],
        };

        VkFramebufferCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = ctx->render_pass,
            .attachmentCount = 1,
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
