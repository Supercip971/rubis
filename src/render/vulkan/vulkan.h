#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <ds/vec.h>
#include <stdio.h>
#include <stdlib.h>

typedef vec_t(const char *) VulkanExts;

// I may split this structure in multiple one, like one for the device etc... But I'm not really trying to make a game engine, so for the moment I don't see the point of doing it.
// Just keep in mind that this structure should be reworked.

typedef struct
{
    VkApplicationInfo app_info;
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkQueue gfx_queue;
    VkQueue present_queue;
    VulkanExts exts;
    VkSwapchainKHR swapchain;
    VkDebugUtilsMessengerEXT debug_messenger;

    VkSurfaceKHR surface;

    uint32_t image_cnt;

} VulkanCtx;

int vulkan_init(VulkanCtx *self, uintptr_t window_handle);

int vulkan_deinit(VulkanCtx *self);

#define vulkan_assert_success$(X)                                          \
    ({                                                                     \
        __auto_type r = (X);                                               \
        if ((r) != VK_SUCCESS)                                             \
        {                                                                  \
            printf(__FILE__ ":l%i "                                        \
                            " " #X " was not successful (error: %i) ! \n", \
                   __LINE__, r);                                           \
            abort();                                                       \
        }                                                                  \
    })

#define vk_try$(x) vulkan_assert_success$(x)
