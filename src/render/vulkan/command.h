#pragma once

#include <ds/vec.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

void vulkan_cmd_pool_init(VulkanCtx *ctx);

void vulkan_cmd_pool_deinit(VulkanCtx *ctx);

void vulkan_cmd_buffer_init(VulkanCtx *ctx);

void vulkan_cmd_buffer_deinit(VulkanCtx *ctx);

void vulkan_record_cmd_buffer(VulkanCtx *ctx, uint32_t img_idx);
