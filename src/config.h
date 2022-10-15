#pragma once

#ifdef NDEBUG
#    define ENABLE_VALIDATION_LAYERS false
#else
#    define ENABLE_VALIDATION_LAYERS true
#endif

#define WINDOW_WIDTH (1920)
#define WINDOW_HEIGHT (1080)

#define USE_GLFW
#define USE_VULKAN

#define CAMERA_CONTROLLABLE true
