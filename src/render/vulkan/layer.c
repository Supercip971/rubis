
#include <ds/vec.h>
#include <render/vulkan/debug.h>
#include <render/vulkan/layer.h>
#include <stdbool.h>
#include <stdio.h>

const char *enabled_layers[] = {
    "VK_LAYER_KHRONOS_validation",
};
static int vulkan_has_layer(const char *name)
{
    bool founded = false;
    uint32_t ext_count = 0;
    vec_t(VkLayerProperties) exts = {};
    vec_init(&exts);

    vkEnumerateInstanceLayerProperties(&ext_count, NULL);

    vec_reserve(&exts, ext_count);
    exts.length = ext_count;
    vkEnumerateInstanceLayerProperties(&ext_count, exts.data);

    for (int i = 0; i < exts.length; i++)
    {
        printf("layer[%i]: %s\n", i, exts.data[i].layerName);
        if (strcmp(name, exts.data[i].layerName) == 0)
        {
            founded = true;
            break;
        }
    }

    vec_deinit(&exts);
    return founded;
}

bool vulkan_load_validation_layer(VkInstanceCreateInfo *create)
{
    for (size_t i = 0; i < sizeof(enabled_layers) / sizeof(enabled_layers[0]); i++)
    {
        if (!vulkan_has_layer(enabled_layers[i]))
        {
            printf("unable to enable layer: %s\n", enabled_layers[i]);
            return false;
        }
    }

    create->enabledLayerCount = sizeof(enabled_layers) / sizeof(enabled_layers[0]);
    create->ppEnabledLayerNames = enabled_layers;

    return true;
}

void vulkan_load_validation_layer_device(VkDeviceCreateInfo *info)
{
    info->enabledLayerCount = sizeof(enabled_layers) / sizeof(enabled_layers[0]);
    info->ppEnabledLayerNames = enabled_layers;
}
