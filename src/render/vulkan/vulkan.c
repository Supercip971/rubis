#include <render/vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <config.h>
#include <ds/vec.h>
#include <render/vulkan/debug.h>
#include <render/vulkan/device.h>
#include <render/vulkan/layer.h>
#include <render/vulkan/logical.h>
#include <render/vulkan/surface.h>
#include <render/vulkan/swapchain.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static int vulkan_dump_extension(void)
{
    uint32_t ext_count = 0;
    vec_t(VkExtensionProperties) exts = {};
    vec_init(&exts);

    vkEnumerateInstanceExtensionProperties(NULL, &ext_count, NULL);

    vec_reserve(&exts, ext_count);
    exts.length = ext_count;
    vkEnumerateInstanceExtensionProperties(NULL, &ext_count, exts.data);

    for (int i = 0; i < exts.length; i++)
    {
        printf("extension[%i]: %s #%i \n", i, exts.data[i].extensionName, exts.data[i].specVersion);
    }

    vec_deinit(&exts);
    return 0;
}

static int vulkan_query_extension(VulkanCtx *self, VkInstanceCreateInfo *info)
{

    uint32_t glfw_ext_count = 0;
    const char **glfw_required_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
    vec_init(&self->exts);

    for (size_t i = 0; i < glfw_ext_count; i++)
    {
        vec_push(&self->exts, glfw_required_extensions[i]);
        printf("required[%li]: %s\n", i, glfw_required_extensions[i]);
    }

    if (ENABLE_VALIDATION_LAYERS)
    {

        vec_push(&self->exts, "VK_EXT_debug_report");
        vec_push(&self->exts, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    vec_push(&self->exts, "VK_KHR_get_surface_capabilities2");
    vec_push(&self->exts, "VK_KHR_xlib_surface");
    vec_push(&self->exts, "VK_KHR_display");
    info->enabledExtensionCount = self->exts.length;
    info->ppEnabledExtensionNames = self->exts.data;

    return 0;
}

int vulkan_create_instance(VulkanCtx *self)
{
    VkInstanceCreateInfo create = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &self->app_info,
    };

    vulkan_query_extension(self, &create);

    create.enabledLayerCount = 0;

    VkDebugUtilsMessengerCreateInfoEXT *debug_info = malloc(sizeof(VkDebugUtilsMessengerCreateInfoEXT));
    if (!debug_info)
    {
        exit(-1);
    }
    memset(debug_info, 0, sizeof(*debug_info));
    vulkan_debug_info(debug_info);
    debug_info->pNext = NULL;

    if (ENABLE_VALIDATION_LAYERS)
    {
        vulkan_load_validation_layer(&create);

        create.pNext = debug_info;
    }

    vk_try$(vkCreateInstance(&create, NULL, &self->instance));

    return 0;
}

static int vulkan_device_init(VulkanCtx *self)
{

    vulkan_pick_physical_device(self);

    vulkan_logical_device_init(self);
    return 0;
}

int vulkan_init(VulkanCtx *self, uintptr_t window_handle)
{
    *self = (VulkanCtx){
        .app_info = (VkApplicationInfo){
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "compute-tracer",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "none",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_MAKE_VERSION(1, 0, 0),
            .pNext = NULL,
        },
        .instance = 0,
    };
    int width, height;
    vulkan_render_surface_target_size(self, window_handle, &width, &height);

    vulkan_dump_extension();

    vulkan_create_instance(self);

    if (ENABLE_VALIDATION_LAYERS)
    {

        vulkan_debug_init(self);
    }
    vulkan_render_surface_init(self, window_handle);

    vulkan_device_init(self);

    vulkan_swapchain_init(self, width, height);

    return 0;
}

int vulkan_deinit(VulkanCtx *self)
{
    vulkan_swapchain_deinit(self);

    vulkan_render_surface_deinit(self);

    vulkan_logical_device_deinit(self);
    if (ENABLE_VALIDATION_LAYERS)
    {
        vulkan_debug_deinit(self);
    }

    vkDestroyInstance(self->instance, NULL);
    vec_deinit(&self->exts);
    return 0;
}
