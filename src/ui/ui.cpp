#include "ui.h"

#ifndef NINC_IMGUI
#include <GLFW/glfw3.h>
#include <thirdparty/imgui/imgui.hpp>
#include <thirdparty/imgui/backends/imgui_impl_glfw.hpp>
#include <thirdparty/imgui/backends/imgui_impl_vulkan.hpp>

extern "C"
{
    
#include <render/vulkan/vulkan.h>

#include <window/window.h>
#include "render/vulkan/command.h"

}
#endif
#include <vulkan/vulkan_core.h>

extern "C" void ui_init(void* window_handle,VulkanCtx* ctx )
{
    ImGui::CreateContext();


	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
	pool_info.pPoolSizes = pool_sizes;

	vk_try$(vkCreateDescriptorPool(ctx->logical_device, &pool_info, NULL, &ctx->gui_pool));

    ImGui_ImplGlfw_InitForVulkan( (GLFWwindow*)window_handle, true);

	struct ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = ctx->instance;
	init_info.PhysicalDevice = ctx->physical_device;
	init_info.Device = ctx->logical_device;
	init_info.Queue = ctx->gfx_queue;
	init_info.DescriptorPool = ctx->gui_pool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, ctx->render_pass);

    VkCommandBuffer cmd = vk_start_single_time_command(ctx);

    ImGui_ImplVulkan_CreateFontsTexture(cmd);

    vk_end_single_time_command(ctx, cmd);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}
extern "C" void ui_event_update(void * event)
{
    (void)event;
}
extern "C" void ui_begin()
{

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();


        //imgui commands
        ImGui::ShowDemoWindow();


}
extern "C" void ui_end()
{

	ImGui::Render();
}

extern "C" void ui_record(VulkanCtx* ctx)
{
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), ctx->cmd_buffer);

}
extern "C" void ui_deinit(VulkanCtx* ctx)
{

		vkDestroyDescriptorPool(ctx->logical_device, ctx->gui_pool, NULL);
		ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
}
