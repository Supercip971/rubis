#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <ds/vec.h>

typedef vec_t(const char *) VulkanExts;

typedef struct
{
    VkApplicationInfo app_info;
    VkInstance instance;
    VkPhysicalDevice device;
    VulkanExts exts;

    VkDebugUtilsMessengerEXT debug_messenger;
} VulkanCtx;

int vulkan_init(VulkanCtx *self);

int vulkan_deinit(VulkanCtx *self);

#define vulkan_assert_success$(X)             \
    if ((X) != VK_SUCCESS)                    \
    {                                         \
        printf(#X "was not successful ! \n"); \
        abort();                              \
    }
