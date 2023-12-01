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


    _Alignas(4) unsigned int bounce_count;
    _Alignas(4) unsigned int scale;
    _Alignas(4) bool use_fsr;

    _Alignas(4) int mesh_id;
    _Alignas(4) int material_offset;
    _Alignas(4) float fov;
} VulkanConstants;

_Static_assert(sizeof(VulkanConstants) < 125, "VulkanConstants is not 256 bytes");

/*
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
*/

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

typedef struct 
{
    vec_t(VulkanTex) textures;

    vec_t(VkDescriptorImageInfo) final_info;
} VulkanTexArrays;
typedef vec_t(VulkanTex) VulkanTexs;


typedef struct __attribute__((packed)) 
{
    _Alignas(16) Vec3 albedo;
    _Alignas(16) Vec3 normal;
    _Alignas(16) Vec3 position;
} PixelInfo;
typedef struct
{
    VkAccelerationStructureGeometryKHR geometries;
    VkAccelerationStructureBuildRangeInfoKHR ranges;

    VkAccelerationStructureKHR handle;

    int requested_size;
    int scratch_size;
    VkAccelerationStructureCreateFlagsKHR flags;
    VkAccelerationStructureBuildGeometryInfoKHR build_info;
    VulkanBuffer buffer;

    VkDeviceAddress handle_addr;
} AccelerationStructure;

typedef vec_t(AccelerationStructure) AccelerationStructures;


typedef struct 
{
    VkApplicationInfo app_info;
    VulkanExts exts;
    
    VkInstance instance;
    int aligned_width;
    int aligned_height;

    // extended debug
    VkDebugUtilsMessengerEXT debug_messenger;
    
    // surface
    VkSurfaceKHR surface;

    // device selection
    VkPhysicalDevice physical_device;
} VulkanCoreCtx;


typedef struct 
{
    VkSwapchainKHR handle;
    VkExtent2D extend;
    VkFormat format;
    uint32_t image_cnt;

    SwapImages images;
    SwapImageViews img_view;
} VulkanSwapchainCtx;

typedef struct 
{
    VkCommandPool command_pool;
    VkQueue queue;
    VkCommandBuffer command_buffer;
} VulkanDevInterface;

typedef struct 
{
    VkPipelineLayout layout;
    VkPipeline handle;
} VulkanPipeline; 
typedef struct 
{
    VkDevice device;
    VulkanSwapchainCtx swapchain;

    VulkanDevInterface gfx;
    VulkanDevInterface compute;

    VulkanPipeline gfx_pipeline;
    VulkanPipeline compute_pipeline;

    VulkanPipeline compute_preview_pipeline;


    VkDescriptorSetLayout descriptor_layout;
    VkDescriptorSet descriptor_set;

    // more abstracted components 

    VulkanTex comp_targ;
    VulkanTex fragment_image;

    // - commands pools- 
    //VkCommandPool cmd_pool;
   // VkCommandPool comp_pool;
    // - queues - 
    //VkQueue gfx_queue;
    //VkQueue comp_queue;
} VulkanGfxCtx;

typedef struct 
{
    
} VulkanAppCtx;
typedef struct
{
    VulkanCoreCtx core;
    VulkanGfxCtx gfx;

    VkQueue present_queue;

   // VkPipelineLayout pipeline_layout;
    //VkPipelineLayout compute_pipeline_layout;



    VkRenderPass render_pass;

    Framebuffers framebuffers;

    VkDescriptorPool gui_pool;



    VkSemaphore image_available_semaphore;
    VkFence compute_fence;

    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    //VulkanCompute compute;


    VulkanBuffer computing_image;

    VulkanBuffer config_buf;

    VulkanBuffer mesh_buf;
    VulkanBuffer emissive_buf;
    VulkanBuffer result_info_buf;


    VulkanBuffer mesh_data_buf;
    VulkanBuffer bvh_buf;

    VulkanBuffer vertex_buffer;

    VkDescriptorPool descriptor_pool;
    uint32_t frame_id;

    VulkanTexArrays combined_textures;
   // VulkanTex _combined_textures;
    VulkanTex skymap;
    VulkanTex frag_targ;
    VulkanTex depth_buffer;

    VkImageView depth_view;

    VulkanTexs overlay_images;

    VulkanTex scene_sampler;
    Vec3 cam_pos;
    Vec3 cam_look;
    Vec3 cam_up;

    bool enable_denoise;

    float cam_aperture;
    float cam_focus_disk;
    Scene scene;
    BvhList bvh_data;
    VkQueryPool qpool;


    int threads_size;

    vec_t(VkQueue) submitting;
    volatile VulkanConstants cfg;

    VkAccelerationStructureKHR tlas;

    VkDeviceAddress tlas_address;

    AccelerationStructures accels;

   // volatile VulkanConfig *cfg;
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
