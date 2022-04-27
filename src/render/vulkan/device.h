#pragma once

#include <ds/vec.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

typedef struct
{
    uint32_t family_idx;
    uint32_t present_family;
    bool _present;
    bool _has_present_family;
} QueueFamilyIndices;

void vulkan_pick_physical_device(VulkanCtx *self);

QueueFamilyIndices vulkan_pick_queue_family(VulkanCtx *self);
