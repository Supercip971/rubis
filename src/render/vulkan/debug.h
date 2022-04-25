#pragma once
#include <render/vulkan/vulkan.h>

void vulkan_debug_init(VulkanCtx *ctx);

void vulkan_debug_deinit(VulkanCtx *ctx);

void vulkan_debug_info(VkDebugUtilsMessengerCreateInfoEXT *info);
