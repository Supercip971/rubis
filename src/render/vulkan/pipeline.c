#include <render/vulkan/pipeline.h>
#include <utils/file.h>
#include <vulkan/vulkan_core.h>
#include "render/vulkan/vertex.h"
#include "render/vulkan/vulkan.h"

VkShaderModule vulkan_shader_create(VulkanCtx *ctx, Buffer code)
{
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.len,
        .pCode = (uint32_t *)code.buffer,
    };

    VkShaderModule module;
    vk_try$(vkCreateShaderModule(ctx->logical_device, &create_info, NULL, &module));
    return module;
}

void vulkan_compute_pipeline(VulkanCtx *ctx)
{
    Buffer comp_code = read_file("build/shaders/comp.spv");
    VkShaderModule comp_mod = vulkan_shader_create(ctx, comp_code);

    VkComputePipelineCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .flags = 0,
        .stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = comp_mod,
            .pName = "main",
        },
        .layout = ctx->pipeline_layout,
    };

    vkCreateComputePipelines(ctx->logical_device, VK_NULL_HANDLE, 1, &info, NULL, &ctx->compute.raw_pipeline);
}


void vulkan_graphics_pipeline_init(VulkanCtx *ctx, VkPipeline* target, VkPipelineLayout* layout ,const char* vpath, const char* fpath)
{
    Buffer frag_code = read_file(fpath);
    Buffer vert_code = read_file(vpath);
    VkShaderModule frag_mod = vulkan_shader_create(ctx, frag_code);
    VkShaderModule vert_mod = vulkan_shader_create(ctx, vert_code);

    VkPipelineShaderStageCreateInfo vcreate_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_mod,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo fcreate_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_mod,
        .pName = "main",
    };

    vec_t(VkPipelineShaderStageCreateInfo) infos;
    vec_init(&infos);
    vec_push(&infos, vcreate_info);
    vec_push(&infos, fcreate_info);

    VertexDescription vert_desc = vulkan_vertex_desc();
    VkVertexInputBindingDescription vert_bind = vulkan_vertex_binding();

    VkPipelineVertexInputStateCreateInfo vt_input = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vert_bind,
        .vertexAttributeDescriptionCount = 5,
        .pVertexAttributeDescriptions = vert_desc.attributes,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)ctx->extend.width,
        .height = (float)ctx->extend.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {0,0},
        .extent = ctx->extend,
    };

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rast_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState color_blending_attachement = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blending_attachement,
        .logicOp = VK_LOGIC_OP_COPY, 
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
        .stencilTestEnable = VK_FALSE,
        .front = {0},
        .back = {0},
    };

    VkPushConstantRange push_constant = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(VulkanConstants),
    };

    VkPipelineLayoutCreateInfo pipeline_create = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &ctx->descriptor_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant,
    };

    vk_try$(vkCreatePipelineLayout(ctx->logical_device, &pipeline_create, NULL, layout));

    VkGraphicsPipelineCreateInfo graphic_pipeline = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = infos.length,
        .flags = 0,
        .pStages = infos.data,
        .pVertexInputState = &vt_input,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rast_create_info,
        .pMultisampleState = &multisampling_create_info,
        .pColorBlendState = &color_blending,
        .pDepthStencilState = &depth_stencil,

        .layout = *layout,
        .renderPass = ctx->render_pass,
        .subpass = 0,
    };

    vk_try$(vkCreateGraphicsPipelines(ctx->logical_device, VK_NULL_HANDLE, 1, &graphic_pipeline, NULL, target));

    vkDestroyShaderModule(ctx->logical_device, frag_mod, NULL);
    vkDestroyShaderModule(ctx->logical_device, vert_mod, NULL);
}


void vulkan_pipeline_init(VulkanCtx *ctx)
{

    vulkan_graphics_pipeline_init(ctx, &ctx->gfx_pipeline, &ctx->pipeline_layout, "build/shaders/vert.spv", "build/shaders/frag.spv");
    vulkan_graphics_pipeline_init(ctx, &ctx->compute_preview_pipeline, &ctx->compute_preview_pipeline_layout, "build/shaders/vert_cview.spv", "build/shaders/frag_cview.spv");


    vulkan_compute_pipeline(ctx);
}

void vulkan_pipeline_deinit(VulkanCtx *ctx)
{
    vkDestroyPipeline(ctx->logical_device, ctx->compute.raw_pipeline, NULL);

    vkDestroyPipeline(ctx->logical_device, ctx->gfx_pipeline, NULL);
    vkDestroyPipeline(ctx->logical_device, ctx->compute_preview_pipeline, NULL);

    vkDestroyPipelineLayout(ctx->logical_device, ctx->pipeline_layout, NULL);
    vkDestroyPipelineLayout(ctx->logical_device, ctx->compute_preview_pipeline_layout, NULL);


    (void)ctx;
}
