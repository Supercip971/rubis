#pragma once 


#include <stddef.h>

typedef struct
{
    void* data;
    size_t len;
} SceneBuffer;


void scene_buffer_init(SceneBuffer* buf);
void scene_buffer_deinit(SceneBuffer* buf);
int scene_buffer_push(SceneBuffer* buf, void* v, size_t len );
