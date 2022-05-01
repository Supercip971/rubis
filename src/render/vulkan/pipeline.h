#pragma once

#include <ds/vec.h>
#include <render/vulkan/vulkan.h>
#include <stdbool.h>

void vulkan_pipeline_init(VulkanCtx *ctx);

void vulkan_pipeline_deinit(VulkanCtx *ctx);
