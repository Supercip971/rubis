#pragma once
#include <ds/vec.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

void vulkan_image_view_init(VulkanGfxCtx *ctx);
void vulkan_create_imageview(VulkanGfxCtx *ctx, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView);

void vulkan_image_view_deinit(VulkanGfxCtx *ctx);
