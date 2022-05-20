
#include <config.h>
#include <render/vulkan/buffer.h>
#include <render/vulkan/command.h>
#include <render/vulkan/device.h>
#include <render/vulkan/logical.h>
void vulkan_cmd_pool_init(VulkanCtx *ctx)
{
    QueueFamilyIndices queue_family_idx = vulkan_pick_queue_family(ctx);

    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_idx.family_idx,
    };

    vk_try$(vkCreateCommandPool(ctx->logical_device, &create_info, NULL, &ctx->cmd_pool));

    VkCommandPoolCreateInfo comp_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_idx.compute_idx,
    };

    vk_try$(vkCreateCommandPool(ctx->logical_device, &comp_create_info, NULL, &ctx->comp_pool));
}

void vulkan_cmd_pool_deinit(VulkanCtx *ctx)
{
    vkDestroyCommandPool(ctx->logical_device, ctx->cmd_pool, NULL);
}

void vulkan_compute_cmd_buffer_record(VulkanCtx *ctx)
{

    vkResetCommandBuffer(ctx->comp_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
    };

    vk_try$(vkBeginCommandBuffer(ctx->comp_buffer, &begin_info));
    {
        //    vkCmdBindPipeline(ctx->comp_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->compute.raw_pipeline);
        vkCmdBindDescriptorSets(ctx->comp_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->pipeline_layout, 0, 1, &ctx->descriptor_set, 0, 0);

        vkCmdDispatch(ctx->comp_buffer, 32, 32, 1);
    }
    vk_try$(vkEndCommandBuffer(ctx->comp_buffer));
}

void vulkan_cmd_buffer_init(VulkanCtx *ctx)
{
    VkCommandBufferAllocateInfo command_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = ctx->cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkAllocateCommandBuffers(ctx->logical_device, &command_alloc_info, &ctx->cmd_buffer);

    VkCommandBufferAllocateInfo compute_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = ctx->comp_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkAllocateCommandBuffers(ctx->logical_device, &compute_alloc_info, &ctx->comp_buffer);

    vulkan_compute_cmd_buffer_record(ctx);
}

void vulkan_record_cmd_buffer(VulkanCtx *ctx, uint32_t img_idx)
{
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    vkResetCommandBuffer(ctx->cmd_buffer, 0);

    vk_try$(vkBeginCommandBuffer(ctx->cmd_buffer, &begin_info));

    VkBufferCopy copy_region = {
        .size = ctx->computing_image.len,
        .dstOffset = 0,
        .srcOffset = 0,
    };

    vkCmdCopyBuffer(ctx->cmd_buffer, ctx->computing_image.buffer, ctx->fragment_image.buffer, 1, &copy_region);

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = ctx->render_pass,
        .framebuffer = ctx->framebuffers.data[img_idx],
        .renderArea = {
            .extent = ctx->extend,
        },
        .clearValueCount = 0,
    };

    vkCmdBeginRenderPass(ctx->cmd_buffer, &render_pass_info, 0);
    {
        vkCmdBindPipeline(ctx->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->gfx_pipeline);

        vkCmdBindDescriptorSets(ctx->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pipeline_layout, 0, 1, &ctx->descriptor_set, 0, NULL);
        vkCmdDraw(ctx->cmd_buffer, 6, 1, 0, 0);
    }
    vkCmdEndRenderPass(ctx->cmd_buffer);

    vk_try$(vkEndCommandBuffer(ctx->cmd_buffer));
}

VkCommandBuffer vk_start_single_time_command(VulkanCtx *ctx)
{
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = ctx->cmd_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd_buf;
    vkAllocateCommandBuffers(ctx->logical_device, &alloc_info, &cmd_buf);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd_buf, &begin_info);

    return cmd_buf;
}
void vk_end_single_time_command(VulkanCtx *ctx, VkCommandBuffer cmd_buf)
{
    vkEndCommandBuffer(cmd_buf);

    VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buf,
    };

    vkQueueSubmit(ctx->gfx_queue, 1, &submit, VK_NULL_HANDLE);

    vkQueueWaitIdle(ctx->gfx_queue);
    vkFreeCommandBuffers(ctx->logical_device, ctx->cmd_pool, 1, &cmd_buf);
}
