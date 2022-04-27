#pragma once
#include <ds/vec.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

// the swapchain is like a collection of framebuffers

typedef vec_t(VkSurfaceFormatKHR) VulkanFormats;
typedef vec_t(VkPresentModeKHR) VulkanPresentModes;

typedef struct
{
    VkSurfaceCapabilitiesKHR capabilities;
    VulkanFormats formats;
    VulkanPresentModes modes;
} SwapChainSupportDetails;

void swap_chain_support_deinit(SwapChainSupportDetails *self);

SwapChainSupportDetails swap_chain_support_query(VulkanCtx *self, VkPhysicalDevice device);

VkSurfaceFormatKHR swap_chain_get_best_format(SwapChainSupportDetails *self);

VkPresentModeKHR swap_chain_get_best_present_mode(SwapChainSupportDetails *self);

VkExtent2D swap_chain_get_best_extent(SwapChainSupportDetails *self, int width, int height);

void vulkan_swapchain_init(VulkanCtx *self, int width, int height);

void vulkan_swapchain_deinit(VulkanCtx *self);
