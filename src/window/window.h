#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t window_id;
} Window;

int window_engine_init(void);

int window_engine_deinit(void);

int window_init(Window *self);

int window_deinit(Window *self);

int window_width(Window *self);

int window_height(Window *self);

bool window_should_close(Window* self);

int window_update(Window* self);
