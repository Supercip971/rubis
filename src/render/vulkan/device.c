#include <ds/vec.h>
#include <render/vulkan/device.h>
#include <render/vulkan/logical.h>
#include <render/vulkan/swapchain.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

static QueueFamilyIndices vulkan_find_queue_family(VulkanCtx *self, VkPhysicalDevice dev)
{
    QueueFamilyIndices idx = {};
    vec_t(VkQueueFamilyProperties) queue_famiy_properties = {};
    vec_init(&queue_famiy_properties);

    uint32_t queue_family_count = 0;
    (vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, NULL));

    if (queue_family_count == 0)
    {
        return idx;
    }

    vec_resize(&queue_famiy_properties, queue_family_count);
    (vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count,
                                              queue_famiy_properties.data));

    for (size_t i = 0; i < queue_family_count; i++)
    {
        VkQueueFamilyProperties curr = queue_famiy_properties.data[i];
        if (curr.queueFlags & VK_QUEUE_GRAPHICS_BIT && curr.queueFlags)
        {
            idx.family_idx = i;
            idx._present = true;
            VkBool32 present_support;

            (vkGetPhysicalDeviceSurfaceSupportKHR(dev, idx.family_idx, self->surface, &present_support));

            if (present_support && glfwGetPhysicalDevicePresentationSupport(self->instance, dev, i))
            {
                idx._has_present_family = true;
                idx.present_family = i;

                break;
            }
            else
            {
                idx._present = false;
            }
        }
        else
        {
            idx._present = false;
        }
    }
    for (size_t i = 0; i < queue_family_count; i++)

    {
        VkQueueFamilyProperties curr = queue_famiy_properties.data[i];
        if (curr.queueFlags & VK_QUEUE_COMPUTE_BIT && idx.family_idx != i)
        {
            idx.compute_idx = i;
            idx._present = true;
            break;
        }
        else
        {
            idx._present = false;
        }
    }

    vec_deinit(&queue_famiy_properties);
    return idx;
}

QueueFamilyIndices vulkan_pick_queue_family(VulkanCtx *self)
{

    QueueFamilyIndices idx = vulkan_find_queue_family(self, self->physical_device);
    return idx;
}

static bool vulkan_is_device_suitable(VulkanCtx *self, VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    QueueFamilyIndices idx = vulkan_find_queue_family(self, device);

    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
        !features.geometryShader ||
        !idx._present ||
        !idx._has_present_family ||
        !vulkan_check_device_extensions(device))
    {
        return false;
    }

    SwapChainSupportDetails swapchain = swap_chain_support_query(self, device);

    if (swapchain.formats.length == 0 || swapchain.modes.length == 0)
    {
        swap_chain_support_deinit(&swapchain);
        return false;
    }

    swap_chain_support_deinit(&swapchain);
    return true;
}

void vulkan_pick_physical_device(VulkanCtx *self)
{
    VkPhysicalDevice device = VK_NULL_HANDLE;
    uint32_t device_count = 0;
    vk_try$(vkEnumeratePhysicalDevices(self->instance, &device_count, NULL));

    if (device_count == 0)
    {
        printf("unable to find vulkan gpu ! \n");
        return;
    }

    vec_t(VkPhysicalDevice) devices = {};
    vec_init(&devices);
    vec_resize(&devices, device_count);

    vk_try$(vkEnumeratePhysicalDevices(self->instance, &device_count, devices.data));

    for (uint32_t i = 0; i < device_count; ++i)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(devices.data[i], &properties);
        printf("device[%u]: %s \n", i, properties.deviceName);
        if (vulkan_is_device_suitable(self, devices.data[i]))
        {
            device = devices.data[i];
            break;
        }
    }

    vec_deinit(&devices);

    if (device == VK_NULL_HANDLE)
    {
        printf("no device founded\n!");
        exit(-1);
        return;
    }
    self->physical_device = device;

    vulkan_pick_queue_family(self);
}
