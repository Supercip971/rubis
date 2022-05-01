#pragma once

#include <ds/vec.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

void vulkan_framebuffer_init(VulkanCtx *ctx);

void vulkan_framebuffer_deinit(VulkanCtx *ctx);
