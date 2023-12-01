#pragma once

#include <render/vulkan/vulkan.h>

int vulkan_render_surface_deinit(VulkanCoreCtx *self);

int vulkan_render_surface_init(VulkanCoreCtx *self, uintptr_t handle);

void vulkan_render_surface_target_size(VulkanCtx *self, uintptr_t handle, int *width, int *height);
