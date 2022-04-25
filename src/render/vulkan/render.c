#include <render/render.h>

#include <stdio.h>
#include <stdint.h>
#include <ds/vec.h>
#include <utils.h>
#include <render/vulkan/vulkan.h>

typedef struct
{
    void* _dummy;
} RenderImpl;

VulkanCtx ctx = {};
vec_t(RenderImpl) impls = {};

int render_engine_init(void)
{
    vulkan_init(&ctx);
    vec_init(&impls);
    return 0;
}

int render_init(Render* self)
{
    *self = (Render){};

    RenderImpl impl = {
        ._dummy = NULL,
    };

    vec_push(&impls, impl);
    return impls.length - 1;
}

int render_deinit(MAYBE_UNUSED Render* self)
{
    return 0;
}

int render_engine_deinit(void)
{
    vec_deinit(&impls);
    vulkan_deinit(&ctx);
    return 0;
}
