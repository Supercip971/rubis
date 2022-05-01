#include <render/vulkan/device.h>
#include <render/vulkan/logical.h>
#include <render/vulkan/swapchain.h>
#include <utils.h>

void swap_chain_support_deinit(SwapChainSupportDetails *self)
{
    vec_deinit(&self->formats);
    vec_deinit(&self->modes);
}

// query modes & formats
SwapChainSupportDetails swap_chain_support_query(VulkanCtx *self, VkPhysicalDevice device)
{
    SwapChainSupportDetails details = {};
    vec_init(&details.modes);
    vec_init(&details.formats);

    printf("%lx dev %lx surface \n", (uintptr_t)device, (uintptr_t)self->surface);
    vulkan_assert_success$(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, self->surface, &details.capabilities));

    uint32_t format_count = 0;
    vulkan_assert_success$(vkGetPhysicalDeviceSurfaceFormatsKHR(device, self->surface, &format_count, NULL));

    if (format_count != 0)
    {
        vec_reserve(&details.formats, format_count);
        details.formats.length = format_count;
        vulkan_assert_success$(vkGetPhysicalDeviceSurfaceFormatsKHR(device, self->surface, &format_count, details.formats.data));
    }
    else
    {
        printf("no format mode! \n");
    }

    uint32_t present_mode_count = 0;
    vulkan_assert_success$(vkGetPhysicalDeviceSurfacePresentModesKHR(device, self->surface, &present_mode_count, NULL));

    if (present_mode_count != 0)
    {
        vec_reserve(&details.modes, present_mode_count);
        details.modes.length = present_mode_count;
        vulkan_assert_success$(vkGetPhysicalDeviceSurfacePresentModesKHR(device, self->surface, &present_mode_count,
                                                                         details.modes.data));
    }
    else
    {
        printf("no present mode! \n");
    }

    return details;
}

VkSurfaceFormatKHR swap_chain_get_best_format(SwapChainSupportDetails *self)
{
    if (self->formats.length == 1 && self->formats.data[0].format == VK_FORMAT_UNDEFINED)
    {
        return (VkSurfaceFormatKHR){
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        };
    }

    for (int i = 0; i < self->formats.length; ++i)
    {
        if (self->formats.data[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            self->formats.data[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return self->formats.data[i];
        }
    }

    return self->formats.data[0];
}

VkPresentModeKHR swap_chain_get_best_present_mode(SwapChainSupportDetails *self)
{

    for (int i = 0; i < self->modes.length; ++i)
    {
        if (self->modes.data[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return self->modes.data[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D swap_chain_get_best_extent(SwapChainSupportDetails *self, int width, int height)
{
    if (self->capabilities.currentExtent.width != UINT32_MAX)
    {
        return self->capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actual_extent = {
            .width = clamp(self->capabilities.minImageExtent.width,
                           self->capabilities.maxImageExtent.width, (uint32_t)width),
            .height = clamp(self->capabilities.minImageExtent.height,
                            self->capabilities.maxImageExtent.height, (uint32_t)height),
        };

        return actual_extent;
    }
}

void vulkan_swapchain_init(VulkanCtx *self, int width, int height)
{
    SwapChainSupportDetails details = swap_chain_support_query(self, self->physical_device);

    VkSurfaceFormatKHR surface_format = swap_chain_get_best_format(&details);
    VkPresentModeKHR present_mode = swap_chain_get_best_present_mode(&details);
    VkExtent2D extent = swap_chain_get_best_extent(&details, width, height);
    self->extend = extent;
    self->swapchain_image_format = surface_format.format;
    self->image_cnt = details.capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = self->surface,
        .minImageCount = self->image_cnt,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = details.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    QueueFamilyIndices indices = vulkan_pick_queue_family(self);
    uint32_t queueFamilyIndices[] = {indices.family_idx, indices.present_family};

    if (indices.family_idx != indices.present_family)
    {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.queueFamilyIndexCount = 0;  // Optional
        swapchain_info.pQueueFamilyIndices = NULL; // Optional
    }

    vulkan_assert_success$(vkCreateSwapchainKHR(self->logical_device, &swapchain_info, NULL, &self->swapchain));

    uint32_t image_cnt = 0;
    vk_try$(vkGetSwapchainImagesKHR(self->logical_device, self->swapchain, &image_cnt, NULL));

    vec_init(&self->swapchain_images);
    vec_reserve(&self->swapchain_images, image_cnt);

    self->swapchain_images.length = image_cnt;
    vk_try$(vkGetSwapchainImagesKHR(self->logical_device, self->swapchain, &image_cnt, self->swapchain_images.data));

    swap_chain_support_deinit(&details);
}

void vulkan_swapchain_deinit(VulkanCtx *self)
{
    vkDestroySwapchainKHR(self->logical_device, self->swapchain, NULL);
}
