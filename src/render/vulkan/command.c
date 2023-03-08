
#include <config.h>
#include <render/vulkan/buffer.h>
#include <render/vulkan/command.h>
#include <render/vulkan/device.h>
#include <render/vulkan/logical.h>
#include <vulkan/vulkan_core.h>
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
        vkCmdBindPipeline(ctx->comp_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->compute.raw_pipeline);

        vkCmdBindDescriptorSets(ctx->comp_buffer,
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                ctx->pipeline_layout, 0, 1, &ctx->descriptor_set, 0, 0);

        vkCmdDispatch(ctx->comp_buffer, ctx->comp_targ.width / ctx->threads_size, ctx->comp_targ.height / ctx->threads_size, 1);
        VkBufferImageCopy region = {
            .bufferImageHeight = 0,
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {.width = ctx->comp_targ.width, .height = ctx->comp_targ.height, .depth = 1},
        };

        vkCmdCopyImageToBuffer(ctx->comp_buffer, ctx->comp_targ.image, ctx->comp_targ.desc_info.imageLayout, ctx->fragment_image.buffer, 1, &region);
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
}

void vulkan_record_cmd_buffer(VulkanCtx *ctx, uint32_t img_idx, bool refresh)
{
    (void)refresh;
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    vkResetCommandBuffer(ctx->cmd_buffer, 0);

    vk_try$(vkBeginCommandBuffer(ctx->cmd_buffer, &begin_info));

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = ctx->render_pass,
        .framebuffer = ctx->framebuffers.data[img_idx],
        .renderArea = {
            .extent = ctx->extend,
        },
        .clearValueCount = 0,
    };

    if (false)
    {

        vkCmdBeginRenderPass(ctx->cmd_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        {

            vkCmdBindPipeline(ctx->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->gfx_pipeline);
            vkCmdBindDescriptorSets(ctx->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pipeline_layout, 0, 1, &ctx->descriptor_set, 0, NULL);

            for (int i = 0; i < ctx->scene.meshes.length; i++)
            {
                Mesh mesh = ctx->scene.meshes.data[i];

                int data_start = mesh.vertices.start;
                int data_end = mesh.vertices.end;

                int data_size = data_end - data_start;

                if (data_size == 0)
                {
                    continue;
                }
                VkBuffer vertex_buffers[] = {ctx->vertex_buffer.buffer};

                VkDeviceSize offsets[] = {data_start * 4 * sizeof(float)};

                vkCmdBindVertexBuffers(ctx->cmd_buffer, 0, 1, vertex_buffers, offsets);
                vkCmdDraw(ctx->cmd_buffer, data_size / SVERTEX_PACKED_COUNT, 1, 0, 0);
            }
            ui_record(ctx);
        }
        vkCmdEndRenderPass(ctx->cmd_buffer);
    }
    else
    {
        vkCmdBeginRenderPass(ctx->cmd_buffer, &render_pass_info, 0);
        {
            vkCmdBindPipeline(ctx->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->gfx_pipeline);

            vkCmdBindDescriptorSets(ctx->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->compute_preview_pipeline_layout, 0, 1, &ctx->descriptor_set, 0, NULL);
            vkCmdDraw(ctx->cmd_buffer, 6, 1, 0, 0);
           ui_record(ctx);
      
        }

        vkCmdEndRenderPass(ctx->cmd_buffer);
    }

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
