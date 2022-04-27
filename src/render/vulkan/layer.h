#pragma once
#include <stdbool.h>
#include <vulkan/vulkan.h>

bool vulkan_load_validation_layer(VkInstanceCreateInfo *create);

void vulkan_load_validation_layer_device(VkDeviceCreateInfo *info);
