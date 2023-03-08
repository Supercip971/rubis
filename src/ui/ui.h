#pragma once 

#include <window/window.h>
#include <render/vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif


void ui_init(void* window_handle,VulkanCtx* ctx );
void ui_event_update(void * event);

void ui_begin();
void ui_end();
void ui_deinit(VulkanCtx* ctx);
void ui_record(VulkanCtx* ctx);
#ifdef __cplusplus
}
#endif
