#include <config.h>
#include <render/vulkan/device.h>
#include <render/vulkan/layer.h>
#include <render/vulkan/logical.h>
// forced include order for clang-format using comments
#include <X11/Xlib-xcb.h>
//
#include <vulkan/vulkan_xcb.h>

const char *device_required_exts[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
};

bool vulkan_check_device_extensions(VkPhysicalDevice device)
{
    uint32_t extensions_count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensions_count, NULL);

    vec_t(VkExtensionProperties) availableExtensions;
    vec_init(&availableExtensions);
    vec_resize(&availableExtensions, extensions_count);

    vkEnumerateDeviceExtensionProperties(device, NULL, &extensions_count, availableExtensions.data);

    for (size_t i = 0; i < sizeof(device_required_exts) / sizeof(device_required_exts[0]); i++)
    {
        bool found = false;
        for (int j = 0; j < availableExtensions.length; j++)
        {

            if (strcmp(device_required_exts[i], availableExtensions.data[j].extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}
void vulkan_logical_device_init(VulkanCtx *ctx)
{
    float priority = 1.0f;
    float priorityb = 0.0f;
    vec_t(VkDeviceQueueCreateInfo) queue_create_infos = {};
    vec_t(uint32_t) queue_create_idx = {};

    vec_init(&queue_create_idx);
    vec_init(&queue_create_infos);

    QueueFamilyIndices idx = vulkan_pick_queue_family(ctx);
    vec_push(&queue_create_idx, idx.family_idx);

    vec_push(&queue_create_idx, idx.compute_idx);

    if (idx._has_present_family && idx.present_family != idx.family_idx)
    {
        vec_push(&queue_create_idx, idx.present_family);
    }

    printf("queue: compute:%i main:%i present:%i\n", idx.compute_idx, idx.family_idx, idx.present_family);

    uint32_t queue_idx = 0;
    int i = 0;

    vec_foreach(&queue_create_idx, queue_idx, i)
    {
        if (queue_idx == idx.compute_idx)
        {
            VkDeviceQueueCreateInfo queue_create_info = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_idx,
                .queueCount = 1,
                .pQueuePriorities = &priority,
            };

            vec_push(&queue_create_infos, queue_create_info);
        }
        else
        {
            VkDeviceQueueCreateInfo queue_create_info = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_idx,
                .queueCount = 1,
                .pQueuePriorities = &priorityb,
            };

            vec_push(&queue_create_infos, queue_create_info);
        }
    }

    VkPhysicalDeviceFeatures features = {};
    
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT feat = 
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
        .runtimeDescriptorArray = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .pNext = NULL,
    };
    features.fragmentStoresAndAtomics = VK_TRUE;
    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queue_create_infos.data,
        .queueCreateInfoCount = queue_create_infos.length,
        .enabledExtensionCount = sizeof(device_required_exts) / sizeof(device_required_exts[0]),
        .ppEnabledExtensionNames = device_required_exts,
        .pEnabledFeatures = NULL,
    };

    VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &feat, 
        .features = features,
    };

    create_info.pNext = &features2;

    vulkan_load_validation_layer_device(&create_info);
    vk_try$(vkCreateDevice(ctx->physical_device, &create_info, NULL, &ctx->logical_device));

    vkGetDeviceQueue(ctx->logical_device, idx.family_idx, 0, &ctx->gfx_queue);
    vkGetDeviceQueue(ctx->logical_device, idx.compute_idx, 0, &ctx->comp_queue);
    vkGetDeviceQueue(ctx->logical_device, idx.present_family, 0, &ctx->present_queue);

    vec_deinit(&queue_create_infos);
    vec_deinit(&queue_create_idx);
}

void vulkan_logical_device_deinit(VulkanCtx *ctx)
{
    vkDestroyDevice(ctx->logical_device, NULL);
}
