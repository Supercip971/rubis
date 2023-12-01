#pragma once

#include "render/vulkan/vulkan.h"
#ifdef NDEBUG
#    define ENABLE_VALIDATION_LAYERS false
#else
#    define ENABLE_VALIDATION_LAYERS true
#endif

#define WINDOW_WIDTH (2560)
#define WINDOW_HEIGHT (1440)

#define USE_GLFW
#define USE_VULKAN


typedef struct Config 
{
    int window_width;
    int window_height;
    bool camera_controllable;
    bool show_ui;
    int rays_bounce;
    bool show_raster;
    int scale_divider;
    bool use_fsr;
    float r_fov;
} Config;

Config get_config();

void set_config(Config cfg);
