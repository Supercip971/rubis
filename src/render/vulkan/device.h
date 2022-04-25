#pragma once

#include <render/vulkan/vulkan.h>
#include <stdbool.h>

typedef struct
{
    uint32_t index;
    bool _present;
} QueueFamilyIndices;

void vulkan_pick_physical_device(VulkanCtx *self);
