#pragma once
#include <render/vulkan/vulkan.h>

int vulkan_render_surface_deinit(VulkanCtx *self);

int vulkan_render_surface_init(VulkanCtx *self, uintptr_t handle);

void vulkan_render_surface_target_size(VulkanCtx *self, uintptr_t handle, int *width, int *height);
