#pragma once

#include <ds/vec.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

void vulkan_cmd_pool_init(VulkanCtx *ctx);

void vulkan_cmd_pool_deinit(VulkanCtx *ctx);

void vulkan_cmd_buffer_init(VulkanGfxCtx *ctx);

void vulkan_cmd_buffer_deinit(VulkanCtx *ctx);

void vulkan_record_cmd_buffer(VulkanCtx *ctx, uint32_t img_idx, bool refresh);

VkCommandBuffer vk_start_single_time_command(VulkanGfxCtx *ctx);

void vk_end_single_time_command(VulkanGfxCtx *ctx, VkCommandBuffer cmd_buf);

void vulkan_compute_cmd_buffer_record(VulkanGfxCtx *ctx, int threads_size, void* constants);
