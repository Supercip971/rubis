#pragma once
#include <render/vulkan/vulkan.h>

void vulkan_debug_init(VulkanCoreCtx *ctx);

void vulkan_debug_deinit(VulkanCoreCtx *ctx);

void vulkan_debug_info(VkDebugUtilsMessengerCreateInfoEXT *info);
