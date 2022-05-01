#pragma once

#include <stdint.h>
#include <window/window.h>

typedef struct
{
    int id;
} Render;

int render_engine_init(Window *window);

int render_engine_deinit(void);

int render_init(Render *self);

int render_deinit(Render *self);

int render_engine_frame(Render *self);

int render_surface_init(Render *self, uintptr_t handle);

int render_surface_deinit(Render *self, uintptr_t handle);
