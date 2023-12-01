#include <log/log.h>
#include <render/vulkan/debug.h>
#include <stdio.h>

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    (void)pUserData;

    printf("[VULKAN] ");
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    {
        printf(BWHT "verbose: ");
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    {
        printf(BGRN "info: ");
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    {
        printf(BYEL "*warn*: ");
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    {
        printf(BRED "**error**: ");
        break;
    }
    default:
    {
        printf("**unknown** %i: ", messageSeverity);
    }
    }

    switch (messageType)
    {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
    {
        printf(BCYN "validation: ");
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
    {
        printf(BMAG "performance:");
        break;
    }
    default:
    {
    }
    }

    printf(CRESET "%s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

static VkResult vulkan_create_debug_messenger(VulkanCoreCtx *self, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(self->instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        return func(self->instance, pCreateInfo, pAllocator, &self->debug_messenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void vulkan_destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void vulkan_debug_deinit(VulkanCoreCtx *ctx)
{
    vulkan_destroy_debug_messenger(ctx->instance, ctx->debug_messenger, NULL);
}

void vulkan_debug_info(VkDebugUtilsMessengerCreateInfoEXT *info)
{
    *info = (VkDebugUtilsMessengerCreateInfoEXT){
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = vulkan_debug_callback,
        .pUserData = NULL,
        .pNext = NULL,
    };
}

void vulkan_debug_init(VulkanCoreCtx *ctx)
{
    VkDebugUtilsMessengerCreateInfoEXT info;
    vulkan_debug_info(&info);
    vk_try$(vulkan_create_debug_messenger(ctx, &info, NULL));
}
