#pragma once
#include <ds/vec.h>
#include <render/vulkan/vulkan.h>

#include <stdbool.h>
#include <vulkan/vulkan_core.h>

void vulkan_scene_textures_init(VulkanCtx *ctx);

void vulkan_scene_textures_deinit(VulkanCtx *ctx);

void vulkan_shader_shared_texture_init(VulkanCtx *ctx, VulkanTex *self, int width, int height, bool fragment);
void swap_image_layout(VulkanCtx *ctx, VkImage image, VkFormat fmt, VkImageLayout old, VkImageLayout new, int layers, bool compute, bool writable);

typedef struct
{
    VkImage image;
    VkImageLayout layout;
    VkDeviceMemory memory;
    VkImageView view;
    uint32_t width;
    uint32_t height;
    uint32_t mips_level;
    uint32_t layer_count;
    VkDescriptorImageInfo descriptor;
    VkSampler sampler;
} VulkanTexture;
