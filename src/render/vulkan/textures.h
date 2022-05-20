#pragma once
#include <ds/vec.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

void vulkan_scene_textures_init(VulkanCtx *ctx);

void vulkan_scene_textures_deinit(VulkanCtx *ctx);

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
