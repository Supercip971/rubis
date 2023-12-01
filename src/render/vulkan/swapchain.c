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
SwapChainSupportDetails swap_chain_support_query(VulkanCoreCtx *self, VkPhysicalDevice device)
{
    SwapChainSupportDetails details = {};
    vec_init(&details.modes);
    vec_init(&details.formats);

    printf("%lx dev %lx surface \n", (uintptr_t)device, (uintptr_t)self->surface);
    vk_try$(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, self->surface, &details.capabilities));

    uint32_t format_count = 0;
    vk_try$(vkGetPhysicalDeviceSurfaceFormatsKHR(device, self->surface, &format_count, NULL));

    if (format_count == 0)
    {
        printf("no format mode! \n");
        return details;
    }

    vec_resize(&details.formats, format_count);

    vk_try$(vkGetPhysicalDeviceSurfaceFormatsKHR(device, self->surface, &format_count, details.formats.data));

    uint32_t present_mode_count = 0;
    vk_try$(vkGetPhysicalDeviceSurfacePresentModesKHR(device, self->surface, &present_mode_count, NULL));

    if (present_mode_count == 0)
    {
        printf("no present mode! \n");
        return details;
    }

    vec_resize(&details.modes, present_mode_count);

    vk_try$(vkGetPhysicalDeviceSurfacePresentModesKHR(device, self->surface, &present_mode_count, details.modes.data));
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
        printf("mode[%i]: %i\n", i, self->modes.data[i]);
    }
    for (int i = 0; i < self->modes.length; ++i)
    {
        if (self->modes.data[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            return self->modes.data[i];
        }
    }

    return VK_PRESENT_MODE_MAILBOX_KHR;
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
    
    //        .width = clamp((uint32_t)width, self->capabilities.minImageExtent.width,
    //                       self->capabilities.maxImageExtent.width),
    //        .height = clamp((uint32_t)height, self->capabilities.minImageExtent.height,
    //                        self->capabilities.maxImageExtent.height ),
        };

        return actual_extent;
    }
}

void vulkan_swapchain_init(VulkanCtx *self, int width, int height)
{
    SwapChainSupportDetails details = swap_chain_support_query(&self->core, self->core.physical_device);

    VkSurfaceFormatKHR surface_format = swap_chain_get_best_format(&details);
    VkPresentModeKHR present_mode = swap_chain_get_best_present_mode(&details);
    VkExtent2D extent = swap_chain_get_best_extent(&details, width, height);
    printf("swapchain extent: %i %i \n", extent.width, extent.height);
    printf("present mode: %i \n", present_mode);
    printf("format: %i-%i\n", surface_format.format, surface_format.colorSpace);
    self->gfx.swapchain.extend = extent;
    self->gfx.swapchain.format = surface_format.format;
    self->gfx.swapchain.image_cnt = details.capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = self->core.surface,
        .minImageCount = self->gfx.swapchain.image_cnt,
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

    QueueFamilyIndices indices = vulkan_pick_queue_family(&self->core);
    uint32_t queueFamilyIndices[] = {indices.family_idx, indices.present_family};

    if (indices.family_idx != indices.present_family)
    {
        printf("concurrent \n");
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        printf("exclusive \n");
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.queueFamilyIndexCount = 0;  // Optional
        swapchain_info.pQueueFamilyIndices = NULL; // Optional
    }

    vk_try$(vkCreateSwapchainKHR(self->gfx.device, &swapchain_info, NULL, &self->gfx.swapchain.handle));

    uint32_t image_cnt = 0;
    vk_try$(vkGetSwapchainImagesKHR(self->gfx.device, self->gfx.swapchain.handle, &image_cnt, NULL));

    vec_init(&self->gfx.swapchain.images);
    vec_resize(&self->gfx.swapchain.images, image_cnt);

    vk_try$(vkGetSwapchainImagesKHR(self->gfx.device, self->gfx.swapchain.handle, &image_cnt, self->gfx.swapchain.images.data));

    swap_chain_support_deinit(&details);
}

void vulkan_swapchain_deinit(VulkanCtx *self)
{
    vkDestroySwapchainKHR(self->gfx.device, self->gfx.swapchain.handle, NULL);
}
