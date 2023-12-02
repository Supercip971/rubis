
#include <config.h>
#include <render/vulkan/buffer.h>
#include <render/vulkan/command.h>
#include <render/vulkan/device.h>
#include <render/vulkan/logical.h>
#include <ui/ui.h>
#include <vulkan/vulkan_core.h>
#include "render/vulkan/vertex.h"
#include "render/vulkan/vulkan.h"
void vulkan_cmd_pool_init(VulkanCtx *ctx)
{
    QueueFamilyIndices queue_family_idx = vulkan_pick_queue_family(&ctx->core);

    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_idx.family_idx,
    };

    vk_try$(vkCreateCommandPool(ctx->gfx.device, &create_info, NULL, &ctx->gfx.gfx.command_pool));

    VkCommandPoolCreateInfo comp_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_idx.compute_idx,
    };

    vk_try$(vkCreateCommandPool(ctx->gfx.device, &comp_create_info, NULL, &ctx->gfx.compute.command_pool));
}

void vulkan_cmd_pool_deinit(VulkanCtx *ctx)
{
    vkDestroyCommandPool(ctx->gfx.device, ctx->gfx.gfx.command_pool, NULL);
}

void vulkan_compute_cmd_buffer_record(VulkanGfxCtx *ctx, int threads_size, void* constants)
{

    vkResetCommandBuffer(ctx->compute.command_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
    };

    vk_try$(vkBeginCommandBuffer(ctx->compute.command_buffer, &begin_info));
    {
        vkCmdBindPipeline(ctx->compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, ctx->compute_pipeline.handle);

        vkCmdBindDescriptorSets(ctx->compute.command_buffer,
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                ctx->compute_pipeline.layout, 0, 1, &ctx->descriptor_set, 0, 0);

        //    vkCmdPushConstants(ctx->cmd_buffer, ctx->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VulkanConstants), (void*)&ctx->cfg);

        vkCmdPushConstants(ctx->compute.command_buffer, ctx->compute_pipeline.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VulkanConstants), (void *)constants);

        int divider = get_config().scale_divider * threads_size;
        vkCmdDispatch(ctx->compute.command_buffer, ctx->comp_targ.width / divider, ctx->comp_targ.height / divider, 1);

        VkImageCopy region = {
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .srcOffset = {0, 0, 0},
            .dstOffset = {0, 0, 0},


            .extent = {.width = ctx->comp_targ.width, .height = ctx->comp_targ.height, .depth = 1},
        };

        vkCmdCopyImage(ctx->compute.command_buffer, ctx->comp_targ.image, ctx->comp_targ.desc_info.imageLayout, ctx->fragment_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        //vkCmdCopyImageToBuffer(ctx->comp_buffer, ctx->comp_targ.image, ctx->comp_targ.desc_info.imageLayout, ctx->fragment_image.buffer, 1, &region);
    }
    vk_try$(vkEndCommandBuffer(ctx->compute.command_buffer));
}

void vulkan_cmd_buffer_init(VulkanGfxCtx *ctx)
{
    VkCommandBufferAllocateInfo command_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = ctx->gfx.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkAllocateCommandBuffers(ctx->device, &command_alloc_info, &ctx->gfx.command_buffer);

    VkCommandBufferAllocateInfo compute_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = ctx->compute.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkAllocateCommandBuffers(ctx->device, &compute_alloc_info, &ctx->compute.command_buffer);
}

void vulkan_record_cmd_buffer(VulkanCtx *ctx, uint32_t img_idx, bool refresh)
{

    (void)refresh;
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    vkResetCommandBuffer(ctx->gfx.gfx.command_buffer, 0);

    vk_try$(vkBeginCommandBuffer(ctx->gfx.gfx.command_buffer, &begin_info));

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = ctx->gfx.render_pass,
        .framebuffer = ctx->gfx.framebuffers.data[img_idx],
        .renderArea = {
            .extent = ctx->gfx.swapchain.extend,
        },
        .clearValueCount = 0,
    };
    VkClearValue clearColor = {
        .color = {
            {0.0f, 0.0f, 0.0f, 1.0f}},
    };
    VkClearValue clearStencil = {

        .depthStencil = {1.0f, 0},
    };

    VkClearValue clearValues[] = {clearColor, clearStencil};
    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clearValues;

    if (get_config().show_raster)
    {

        vkCmdBeginRenderPass(ctx->gfx.gfx.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        {

            vkCmdBindPipeline(ctx->gfx.gfx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->gfx.gfx_pipeline.handle);
            vkCmdBindDescriptorSets(ctx->gfx.gfx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->gfx.gfx_pipeline.layout, 0, 1, &ctx->gfx.descriptor_set, 0, NULL);

            for (int i = 0; i < ctx->scene.data.meshes.length; i++)
            {
                Mesh mesh = ctx->scene.data.meshes.data[i];

                int data_start = mesh.vertices.start;
                int data_end = mesh.vertices.end;

                int data_size = data_end - data_start;

                if (data_size == 0)
                {
                    continue;
                }
                VkBuffer vertex_buffers[] = {ctx->vertex_buffer.buffer};

                VkDeviceSize offsets[] = {data_start * 4 * sizeof(float)};

                VulkanConstants node = ctx->cfg;
                node.mesh_id = i;
                node.material_offset = mesh.material.start;
                vkCmdPushConstants(ctx->gfx.gfx.command_buffer, ctx->gfx.gfx_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VulkanConstants), &node);
                vkCmdBindVertexBuffers(ctx->gfx.gfx.command_buffer, 0, 1, vertex_buffers, offsets);
                vkCmdDraw(ctx->gfx.gfx.command_buffer, data_size / SVERTEX_PACKED_COUNT, 1, 0, 0);
            }

            if(get_config().show_ui)
            {
                ui_record(ctx);
            }
        }
        vkCmdEndRenderPass(ctx->gfx.gfx.command_buffer);
    }
    else
    {
        vkCmdBeginRenderPass(ctx->gfx.gfx.command_buffer, &render_pass_info, 0);
        {
            vkCmdBindPipeline(ctx->gfx.gfx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->gfx.compute_preview_pipeline.handle);

            vkCmdPushConstants(ctx->gfx.gfx.command_buffer, ctx->gfx.compute_preview_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VulkanConstants), (void *)&ctx->cfg);
            vkCmdBindDescriptorSets(ctx->gfx.gfx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->gfx.compute_preview_pipeline.layout, 0, 1, &ctx->gfx.descriptor_set, 0, NULL);
            vkCmdDraw(ctx->gfx.gfx.command_buffer, 6, 1, 0, 0);
 
            if(get_config().show_ui)
            {
                ui_record(ctx);
            }     
        }

        vkCmdEndRenderPass(ctx->gfx.gfx.command_buffer);
    }

    vk_try$(vkEndCommandBuffer(ctx->gfx.gfx.command_buffer));
}

VkCommandBuffer vk_start_single_time_command(VulkanGfxCtx *ctx)
{
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = ctx->gfx.command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd_buf;
    vkAllocateCommandBuffers(ctx->device, &alloc_info, &cmd_buf);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd_buf, &begin_info);

    return cmd_buf;
}
void vk_end_single_time_command(VulkanGfxCtx *ctx, VkCommandBuffer cmd_buf)
{
    vkEndCommandBuffer(cmd_buf);

    VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buf,
    };

    vkQueueSubmit(ctx->gfx.queue, 1, &submit, VK_NULL_HANDLE);

    vkQueueWaitIdle(ctx->gfx.queue);
    vkFreeCommandBuffers(ctx->device, ctx->gfx.command_pool, 1, &cmd_buf);
}
