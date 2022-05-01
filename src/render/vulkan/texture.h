#pragma once
#include <vulkan/vulkan.h>

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
