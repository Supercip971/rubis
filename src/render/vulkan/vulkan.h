#pragma once

#include "obj/img.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <ds/vec.h>
#include <math/vec3.h>
#include <obj/scene.h>
#include <stdio.h>
#include <stdlib.h>

typedef vec_t(const char *) VulkanExts;
typedef vec_t(VkImage) SwapImages;
typedef vec_t(VkImageView) SwapImageViews;
typedef vec_t(VkFramebuffer) Framebuffers;

#include <obj/bvh.h>
// I may split this structure in multiple one, like one for the device etc... But I'm not really trying to make a game engine, so for the moment I don't see the point of doing it.
// Just keep in mind that this structure should be reworked.
typedef struct
{
    size_t len;
    void *data;
    VkBuffer buffer;
    VkDeviceMemory raw_memory;
} VulkanBuffer;

typedef struct
{
    _Alignas(4) float width;
    _Alignas(4) float height;
    _Alignas(4) unsigned int t;

    _Alignas(16) Vec3 cam_pos;
    _Alignas(16) Vec3 cam_look;
    _Alignas(16) Vec3 cam_up;
    _Alignas(4) float aperture;
    _Alignas(4) float focus_disc;

    _Alignas(16) float proj_matrix[4][4];
    _Alignas(16) float view_matrix[4][4];

    _Alignas(4) unsigned int bounce_count;

} VulkanConfig;

typedef struct
{
    VkQueue queue;
    VkCommandPool cmd_pool;
    VkCommandBuffer cmd_buffer;
    VkSemaphore smeaphore;
    VkDescriptorSetLayout layout;
    VkDescriptorSet descriptor;
    VkPipelineLayout pipelines;
    VkPipeline raw_pipeline;
    int32_t pipeline_idx;
} VulkanCompute;

typedef struct
{
    VkDescriptorImageInfo desc_info;
    VkImage image;
    VkDeviceMemory mem;
    int width;
    int height;
    VkFormat fmt;
    VkImageView view; // optional
} VulkanTex;

typedef vec_t(VulkanTex) VulkanTexs;

typedef struct
{
    VkApplicationInfo app_info;
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkQueue gfx_queue;
    VkQueue comp_queue;

    VkQueue present_queue;
    VulkanExts exts;
    VkSwapchainKHR swapchain;
    VkDebugUtilsMessengerEXT debug_messenger;

    VkSurfaceKHR surface;

    uint32_t image_cnt;

    SwapImages swapchain_images;
    SwapImageViews swapchain_img_view;

    VkExtent2D extend;
    VkFormat swapchain_image_format;
    VkPipelineLayout pipeline_layout;

    VkPipeline gfx_pipeline;
    VkPipelineLayout compute_preview_pipeline_layout;


    VkPipeline compute_preview_pipeline;
    VkPipeline compute_pipeline;

    VkRenderPass render_pass;

    Framebuffers framebuffers;

    VkCommandPool cmd_pool;
    VkCommandPool comp_pool;
    VkDescriptorPool gui_pool;


    VkCommandBuffer cmd_buffer;
    VkCommandBuffer comp_buffer;

    VkSemaphore image_available_semaphore;
    VkFence compute_fence;

    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    VulkanCompute compute;

    VkDescriptorSetLayout descriptor_layout;
    VkDescriptorSet descriptor_set;

    VulkanBuffer computing_image;
    VulkanBuffer fragment_image;

    VulkanBuffer config_buf;

    VulkanBuffer mesh_buf;

    VulkanBuffer mesh_data_buf;
    VulkanBuffer bvh_buf;

    VulkanBuffer vertex_buffer;

    VkDescriptorPool descriptor_pool;
    uint32_t frame_id;

    VulkanTex combined_textures;
    VulkanTex skymap;
    VulkanTex comp_targ;
    VulkanTex frag_targ;
    VulkanTex depth_buffer;

    VkImageView depth_view;

    VulkanTexs overlay_images;

    Vec3 cam_pos;
    Vec3 cam_look;
    Vec3 cam_up;

    bool enable_denoise;

    float cam_aperture;
    float cam_focus_disk;
    Scene scene;
    BvhList bvh_data;
    VkQueryPool qpool;


    int aligned_width;
    int aligned_height;
    int threads_size;

    vec_t(VkQueue) submitting;
    volatile VulkanConfig *cfg;
} VulkanCtx;

int vulkan_init(VulkanCtx *self, uintptr_t window_handle, Scene *scene);

int vulkan_frame(VulkanCtx *self);

int vulkan_deinit(VulkanCtx *self);

#define vulkan_assert_success$(X)                                          \
    ({                                                                     \
        __auto_type r = (X);                                               \
        if ((r) != VK_SUCCESS)                                             \
        {                                                                  \
            printf(__FILE__ ":l%i "                                        \
                            " " #X " was not successful (error: %i) ! \n", \
                   __LINE__, r);                                           \
            abort();                                                       \
        }                                                                  \
    })

#define vk_try$(x) vulkan_assert_success$(x)
