#pragma once
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

void vulkan_device_init(VulkanCtx *ctx);

void vulkan_device_deinit(VulkanCtx *ctx);

bool vulkan_check_device_extensions(VkPhysicalDevice device);
