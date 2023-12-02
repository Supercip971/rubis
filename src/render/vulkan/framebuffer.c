#include <render/vulkan/framebuffer.h>

void vulkan_framebuffer_init(VulkanCtx *ctx)
{
    int fb_count = ctx->gfx.swapchain.img_view.length;
    vec_init(&ctx->gfx.framebuffers);

    vec_resize(&ctx->gfx.framebuffers, fb_count);

    for (int i = 0; i < fb_count; i++)
    {
        VkImageView attachement[] = {
            ctx->gfx.swapchain.img_view.data[i],
            ctx->gfx.depth_view
        };

        VkFramebufferCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = ctx->gfx.render_pass,
            .attachmentCount = 2,
            .pAttachments = attachement,
            .width = ctx->gfx.swapchain.extend.width,
            .height = ctx->gfx.swapchain.extend.height,
            .layers = 1,
        };

        vk_try$(vkCreateFramebuffer(ctx->gfx.device, &create_info, NULL, &ctx->gfx.framebuffers.data[i]));
    }
}

void vulkan_framebuffer_deinit(VulkanCtx *ctx)
{
    int fb_count = ctx->gfx.swapchain.img_view.length;

    for (int i = 0; i < fb_count; i++)
    {
        vkDestroyFramebuffer(ctx->gfx.device, ctx->gfx.framebuffers.data[i], NULL);
    }
    vec_deinit(&ctx->gfx.framebuffers);
}
