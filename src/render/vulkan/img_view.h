#pragma once
#include <ds/vec.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

void vulkan_image_view_init(VulkanCtx *ctx);
void vulkan_create_imageview(VulkanCtx *ctx, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView);

void vulkan_image_view_deinit(VulkanCtx *ctx);
