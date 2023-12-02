#include "ui.h"

#ifndef NINC_IMGUI
#    include <GLFW/glfw3.h>
#    include <thirdparty/imgui/backends/imgui_impl_glfw.hpp>
#    include <thirdparty/imgui/backends/imgui_impl_vulkan.hpp>
#    include <thirdparty/imgui/imgui.hpp>


extern "C"
{

#include "config.h"
#    include <render/vulkan/vulkan.h>
#    include <window/window.h>
#    include "render/vulkan/command.h"
}
#endif
#include <vulkan/vulkan_core.h>

extern "C" void ui_init(void *window_handle, VulkanCtx *ctx)
{
    ImGui::CreateContext();

    VkDescriptorPoolSize pool_sizes[] =
        {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
    pool_info.pPoolSizes = pool_sizes;

    vk_try$(vkCreateDescriptorPool(ctx->gfx.device, &pool_info, NULL, &ctx->gui_pool));

    ImGui_ImplGlfw_InitForVulkan((GLFWwindow *)window_handle, true);

    struct ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = ctx->core.instance;
    init_info.PhysicalDevice = ctx->core.physical_device;
    init_info.Device = ctx->gfx.device;
    init_info.Queue = ctx->gfx.gfx.queue;
    init_info.DescriptorPool = ctx->gui_pool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, ctx->gfx.render_pass);

    VkCommandBuffer cmd = vk_start_single_time_command(&ctx->gfx);

    ImGui_ImplVulkan_CreateFontsTexture(cmd);

    vk_end_single_time_command(&ctx->gfx, cmd);

    ImGui_ImplVulkan_DestroyFontUploadObjects();

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
}
extern "C" void ui_event_update(void *event)
{
    (void)event;
}
extern "C" void ui_begin(UiInfo info)
{

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Raytracing configs", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }
    ImGui::Text("press 'H' to hide the window");
   	ImGui::Text("Accumulated frames: %d", info.frame);
   	ImGui::Text("Accumulated frame per second: %f per second", info.fps);

	Config current = get_config();
	float mrays = (float)current.rays_bounce * (float)info.fps * (float)info.width * (float)info.height / 1000000.0f;

	ImGui::Text("(guessed) MRays per second: %f", mrays);




 	ImGui::Checkbox("Enable camera controls (press C)", &current.camera_controllable);

    ImGui::SliderInt("Rays bounce", &current.rays_bounce, 1, 12);
    ImGui::SliderInt("Render divider", &current.scale_divider, 1, 8);

    ImGui::SliderFloat("Rasterizer fov", &current.r_fov, 10.f, 90.f);
    
 	ImGui::Checkbox("Use amd FSR", &current.use_fsr);



    ImGui::Checkbox("Show rasterizer (no raytrace)", &current.show_raster);


	set_config(current);
	
	ImGui::End();
            
}
extern "C" void ui_end()
{

    ImGui::Render();
}

extern "C" void ui_record(VulkanCtx *ctx)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), ctx->gfx.gfx.command_buffer);
}
extern "C" void ui_deinit(VulkanCtx *ctx)
{

    vkDestroyDescriptorPool(ctx->gfx.device, ctx->gui_pool, NULL);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
