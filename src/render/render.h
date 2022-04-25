#pragma once 


typedef struct
{
    int id;
} Render;

int render_engine_init(void);

int render_engine_deinit(void);

int render_init(Render* self);

int render_deinit(Render* self);
