#pragma once
#include <render/vulkan/vulkan.h>

void vulkan_desc_set_layout(VulkanCtx *ctx);

void vulkan_desc_layout_deinit(VulkanCtx *ctx);
void vulkan_desc_create_pool(VulkanCtx *ctx);
