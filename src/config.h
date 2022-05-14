#pragma once

#ifdef NDEBUG
#    define ENABLE_VALIDATION_LAYERS true
#else
#    define ENABLE_VALIDATION_LAYERS true
#endif

#define WINDOW_WIDTH (480 * 4)
#define WINDOW_HEIGHT (270 * 4)

#define USE_GLFW
#define USE_VULKAN

#define CAMERA_CONTROLLABLE false
