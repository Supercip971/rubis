#pragma once

#include <render/vulkan/vulkan.h>
VulkanBuffer vk_buffer_alloc(VulkanCtx *ctx, size_t len, VkBufferUsageFlags flags, VkMemoryPropertyFlags properties);
void *vk_buffer_map(VulkanCtx *ctx, VulkanBuffer with);
void vk_buffer_unmap(VulkanCtx *ctx, VulkanBuffer with);
void vk_buffer_copy(VulkanCtx *ctx, VulkanBuffer to, VulkanBuffer from);
void vk_buffer_free(VulkanCtx *ctx, VulkanBuffer buffer);
void vk_buffer_set(VulkanCtx *ctx, VulkanBuffer buf, uint32_t data);
