#pragma once 

#include <window/window.h>
#include <render/vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct 
{
    float fps;
    int frame;
    int width;
    int height;
} UiInfo;
void ui_init(void* window_handle,VulkanCtx* ctx );
void ui_event_update(void * event);

void ui_begin(UiInfo info);
void ui_end();
void ui_deinit(VulkanCtx* ctx);
void ui_record(VulkanCtx* ctx);
#ifdef __cplusplus
}
#endif
