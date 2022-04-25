#pragma once
#include <vulkan/vulkan.h>
#include <stdbool.h>

bool vulkan_load_validation_layer(VkInstanceCreateInfo* create);
