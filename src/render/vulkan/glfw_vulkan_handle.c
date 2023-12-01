#define VK_USE_PLATFORM_XLIB_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>
#include <render/vulkan/vulkan.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int vulkan_render_surface_init(VulkanCoreCtx *self, uintptr_t handle)
{
    VkSurfaceKHR surface = 0;

    vk_try$(glfwCreateWindowSurface(self->instance, (GLFWwindow *)handle, NULL, &surface));
    self->surface = surface;
    return 0;
}

int vulkan_render_surface_deinit(VulkanCoreCtx *self)
{
    vkDestroySurfaceKHR(self->instance, self->surface, NULL);
    return 0;
}

void vulkan_render_surface_target_size(VulkanCtx *self, uintptr_t handle, int *width, int *height)
{
    (void)self;
    glfwGetFramebufferSize((GLFWwindow *)handle, width, height);
}
