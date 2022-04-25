#include <ds/vec.h>
#include <render/vulkan/device.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

static QueueFamilyIndices vulkan_find_queue_family(VkPhysicalDevice dev)
{
    QueueFamilyIndices idx = (QueueFamilyIndices){.index = 0, ._present = false};

    vec_t(VkQueueFamilyProperties) queue_famiy_properties = {};
    vec_init(&queue_famiy_properties);

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, NULL);

    vec_reserve(&queue_famiy_properties, queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, queue_famiy_properties.data);

    for (size_t i = 0; i < queue_family_count; i++)
    {
        VkQueueFamilyProperties curr = queue_famiy_properties.data[i];
        if (curr.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            idx.index = i;
            idx._present = true;
        }
    }

    vec_deinit(&queue_famiy_properties);
    return idx;
}

static QueueFamilyIndices vulkan_pick_queue_family(VulkanCtx *self)
{
    return vulkan_find_queue_family(self->device);
}

static bool vulkan_is_device_suitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    QueueFamilyIndices idx = vulkan_find_queue_family(device);

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           features.geometryShader && idx._present;
}

void vulkan_pick_physical_device(VulkanCtx *self)
{
    VkPhysicalDevice device = VK_NULL_HANDLE;
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(self->instance, &device_count, NULL);

    if (device_count == 0)
    {
        printf("unable to find vulkan gpu ! \n");
        return;
    }

    vec_t(VkPhysicalDevice) devices = {};
    vec_init(&devices);
    vec_reserve(&devices, device_count);
    vkEnumeratePhysicalDevices(self->instance, &device_count, devices.data);

    for (uint32_t i = 0; i < device_count; ++i)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(devices.data[i], &properties);
        printf("device[%u]: %s \n", i, properties.deviceName);
        if (vulkan_is_device_suitable(devices.data[i]))
        {
            device = devices.data[i];
            break;
        }
    }

    vec_deinit(&devices);

    self->device = device;

    vulkan_pick_queue_family(self);
}
