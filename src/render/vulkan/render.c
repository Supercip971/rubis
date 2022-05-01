#include <config.h>
#include <ds/vec.h>
#include <render/render.h>
#include <render/vulkan/surface.h>
#include <render/vulkan/vulkan.h>
#include <stdint.h>
#include <stdio.h>
#include <utils.h>
#include <window/window.h>

typedef struct
{
    void *_dummy;
} RenderImpl;

VulkanCtx ctx = {};
vec_t(RenderImpl) impls = {};

int render_surface_init(Render *self, uintptr_t handle)
{
    (void)self;
    return vulkan_render_surface_init(&ctx, handle);
}
int render_surface_deinit(Render *self, uintptr_t handle)
{
    (void)self;
    (void)handle;
    return vulkan_render_surface_deinit(&ctx);
}

int render_engine_init(Window *window)
{
    vulkan_init(&ctx, window_raw_handle(window));
    vec_init(&impls);
    return 0;
}

int render_init(Render *self)
{
    *self = (Render){};

    RenderImpl impl = {
        ._dummy = NULL,
    };

    vec_push(&impls, impl);
    return impls.length - 1;
}

int render_engine_frame(Render *self)
{
    (void)self;
    vulkan_frame(&ctx);
    return 0;
}

int render_deinit(MAYBE_UNUSED Render *self)
{
    return 0;
}

int render_engine_deinit(void)
{
    vulkan_deinit(&ctx);

    vec_deinit(&impls);
    return 0;
}
