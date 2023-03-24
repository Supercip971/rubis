#include <render/vulkan/vulkan.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include "math/mat4.h"
#include "render/vulkan/depth.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <config.h>
#include <ds/vec.h>
#include <render/vulkan/buffer.h>
#include <render/vulkan/command.h>
#include <render/vulkan/debug.h>
#include <render/vulkan/desc_set_layout.h>
#include <render/vulkan/device.h>
#include <render/vulkan/framebuffer.h>
#include <render/vulkan/img_view.h>
#include <render/vulkan/layer.h>
#include <render/vulkan/logical.h>
#include <render/vulkan/vertex_buf.h>
#include <render/vulkan/pipeline.h>
#include <render/vulkan/render_pass.h>
#include <render/vulkan/surface.h>
#include <render/vulkan/swapchain.h>
#include <render/vulkan/sync.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "textures.h"
#include <ui/ui.h>

static int vulkan_dump_extension(void)
{
    uint32_t ext_count = 0;
    vec_t(VkExtensionProperties) exts = {};
    vec_init(&exts);

    vkEnumerateInstanceExtensionProperties(NULL, &ext_count, NULL);

    vec_resize(&exts, ext_count);

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
        vec_push(&self->exts, "VK_EXT_validation_features");
        vec_push(&self->exts, "VK_EXT_debug_utils");
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

    VkValidationFeatureEnableEXT enables[] = {};
    VkValidationFeaturesEXT features = {};

    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.enabledValidationFeatureCount = 0;
    features.pEnabledValidationFeatures = (enables);
    create.pNext = &features;
    vulkan_query_extension(self, &create);

    create.enabledLayerCount = 0;

    VkDebugUtilsMessengerCreateInfoEXT *debug_info = malloc(sizeof(VkDebugUtilsMessengerCreateInfoEXT));
    if (!debug_info)
    {
        exit(-1);
    }
    *debug_info = (VkDebugUtilsMessengerCreateInfoEXT){};

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

struct timespec start;

int vulkan_init(VulkanCtx *self, uintptr_t window_handle, Scene *scene)
{
    *self = (VulkanCtx){
        .app_info = (VkApplicationInfo){
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "compute-tracer",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "none",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3,
            .pNext = NULL,
        },
        .instance = 0,
        .frame_id = 0,
        .threads_size = 4,
        .scene = *scene};

    printf("loading bvh %i...\n", scene->meshes.length);
    bvh_init(&self->bvh_data, scene);
    printf("loaded bvh\n");

    int width = 0, height = 0;
    vulkan_render_surface_target_size(self, window_handle, &width, &height);
    self->aligned_width = (int)(floor((float)width / (float)self->threads_size) * self->threads_size);
    self->aligned_height = (int)(floor((float)height / (float)self->threads_size) * self->threads_size);

    vulkan_dump_extension();

    vulkan_create_instance(self);

    if (ENABLE_VALIDATION_LAYERS)
    {
        vulkan_debug_init(self);
    }

    vulkan_render_surface_init(self, window_handle);

    vulkan_device_init(self);

    vulkan_swapchain_init(self, width, height);

    vulkan_image_view_init(self);

    vulkan_cmd_pool_init(self);

    vulkan_cmd_buffer_init(self);

    vulkan_depth_target_init(self);

    vulkan_render_pass_init(self);
    vulkan_scene_textures_init(self);

    vulkan_vertex_buffer_init(self);
    vulkan_shader_shared_texture_init(self, &self->comp_targ, self->aligned_width, self->aligned_height, false);
    vulkan_shader_shared_texture_init(self, &self->frag_targ, self->aligned_width, self->aligned_height, true);

    vulkan_desc_set_layout(self);

    vulkan_pipeline_init(self);
    vulkan_compute_cmd_buffer_record(self);
    vulkan_framebuffer_init(self);

    vulkan_sync_init(self);
    scene_buf_value_init(self);

    ui_init((void*)window_handle, self);
    clock_gettime(CLOCK_REALTIME, &start);

   // self->cfg = vk_buffer_map(self, self->config_buf);

    return 0;
}

int vulkan_deinit(VulkanCtx *self)
{
    vkDeviceWaitIdle(self->logical_device);
    ui_deinit(self);
    vk_buffer_unmap(self, self->config_buf);
    vulkan_sync_deinit(self);
    vulkan_vertex_buffer_deinit(self);

    vulkan_depth_target_deinit(self);
    vulkan_cmd_pool_deinit(self);
    vulkan_framebuffer_deinit(self);

    vulkan_pipeline_deinit(self);
    vulkan_render_pass_deinit(self);

    vulkan_image_view_deinit(self);

    vulkan_swapchain_deinit(self);
    vulkan_desc_layout_deinit(self);

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

float v = 0;

int tick = 0;
float fps = 0;
int vulkan_frame(VulkanCtx *self)
{

    UiInfo info = {
        .fps = fps,
        .frame = tick,
        .width = self->aligned_width,
        .height = self->aligned_height,
    };

    ui_begin(info);

    ui_end();
    Vec3 cam_look = vec3_add(self->cam_look, self->cam_pos);

    if (!vec3_eq(self->cfg.cam_pos, self->cam_pos) ||
        !vec3_eq(self->cfg.cam_look, cam_look))
    {
        self->frame_id = 0;
    }

    self->cfg.bounce_count = (get_config().rays_bounce);
    self->cfg.cam_pos = self->cam_pos;
    self->cfg.focus_disc = self->cam_focus_disk;
    self->cfg.aperture = self->cam_aperture;
    self->cfg.cam_look = cam_look;
    self->cfg.scale = get_config().scale_divider;
    self->cfg.use_fsr = get_config().use_fsr;



    self->cfg.width = self->aligned_width;
    self->cfg.height = self->aligned_height;
    self->cfg.t = self->frame_id;

   // Matrix4x4 view = matrix_lookat(self->cam_pos, cam_look, vec3$(0,1,0));

   // Matrix4x4 proj = matrix_perspective(get_config().r_fov, (float)self->aligned_width / (float)self->aligned_height, 0.001, 1000);

    for(int x = 0; x < 4; x++)
    {
        for(int y = 0; y < 4; y++)
        {
         //   self->cfg.view_matrix[x][y]= view.value[x][y];
         //   self->cfg.proj_matrix[x][y]= proj.value[x][y];
        }
    }
#if 1

    if (vkGetFenceStatus(self->logical_device, self->compute_fence) == VK_SUCCESS)
    {

        //     VkCommandBuffer cmd = vk_start_single_time_command(self);
        /*  VkImageCopy region = {

              .srcSubresource = {
                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                  .layerCount = 1,
              },
              .srcOffset = {},
              .dstSubresource = {
                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                  .layerCount = 1,
              },
              .dstOffset = {},
              .extent = {
                  .width = self->comp_targ.width,
                  .height = self->comp_targ.height,
                  .depth = 1,
              },
          };*/
        //  vkCmdCopyImage(cmd, self->comp_targ.image, self->comp_targ.desc_info.imageLayout, self->frag_targ.image, self->frag_targ.desc_info.imageLayout, 1, &region);

        //    vk_end_single_time_command(self, cmd);
        struct timespec cur;
        clock_gettime(CLOCK_REALTIME, &cur);
        double accum;
        accum = (cur.tv_sec - start.tv_sec) + (double)(cur.tv_nsec - start.tv_nsec) / (double)1000000000L;

        fps = 1 / accum;
        tick = self->frame_id;
       // printf("a1 %lf %i \n", 1 / accum, self->frame_id);

        vkResetFences(self->logical_device, 1, &self->compute_fence);

        vulkan_compute_cmd_buffer_record(self);

        VkSubmitInfo submitInfo2 = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &self->comp_buffer,
        };

        vk_try$(vkQueueSubmit(self->comp_queue, 1, &submitInfo2, self->compute_fence));

        self->frame_id += 1;
       // printf("a2\n");

        clock_gettime(CLOCK_REALTIME, &start);

    }

#endif
    if (vkGetFenceStatus(self->logical_device, self->in_flight_fence) == VK_SUCCESS)
    {
        vk_try$(vkResetFences(self->logical_device, 1, &self->in_flight_fence));

        uint32_t image_idx = 0;

        vk_try$(vkAcquireNextImageKHR(self->logical_device, self->swapchain, UINT64_MAX, self->image_available_semaphore, VK_NULL_HANDLE, &image_idx));
        vulkan_record_cmd_buffer(self, image_idx, true);
        VkSemaphore signaledSemaphores[] = {self->render_finished_semaphore};
        VkSemaphore waitSemaphores[] = {self->image_available_semaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &self->cmd_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signaledSemaphores,
        };

        vk_try$(vkQueueSubmit(self->gfx_queue, 1, &submitInfo, self->in_flight_fence));

        VkSwapchainKHR swapchains[] = {self->swapchain};
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signaledSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapchains,
            .pImageIndices = &image_idx,

        };
        vk_try$(vkQueuePresentKHR(self->present_queue, &present_info));
    }
    else if(!get_config().show_raster)
    {
        vkWaitForFences(self->logical_device, 1, &self->compute_fence, VK_TRUE, 16666000);
    }
    return 0;
}
