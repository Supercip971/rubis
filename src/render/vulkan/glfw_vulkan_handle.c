#define VK_USE_PLATFORM_XLIB_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <render/vulkan/vulkan.h>
#include <stdint.h>

VkXlibSurfaceCreateInfoKHR glfw_vulkan_handle(GLFWwindow *window)
{
    VkXlibSurfaceCreateInfoKHR sinfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = glfwGetX11Display(),
        .window = glfwGetX11Window(window),
        .pNext = NULL,
    };
    return sinfo;
}
int vulkan_render_surface_init(VulkanCtx *self, uintptr_t handle)
{
    VkXlibSurfaceCreateInfoKHR sinfo = glfw_vulkan_handle((GLFWwindow *)handle);
    vulkan_assert_success$(vkCreateXlibSurfaceKHR(self->instance, &sinfo, NULL, &self->surface));

    //    vulkan_assert_success$(glfwCreateWindowSurface(self->instance, (GLFWwindow *)handle, NULL, &self->surface));
    return 0;
}
int vulkan_render_surface_deinit(VulkanCtx *self)
{

    vkDestroySurfaceKHR(self->instance, self->surface, NULL);
    return 0;
}

void vulkan_render_surface_target_size(VulkanCtx *self, uintptr_t handle, int *width, int *height)
{
    (void)self;
    glfwGetFramebufferSize((GLFWwindow *)handle, width, height);
}
