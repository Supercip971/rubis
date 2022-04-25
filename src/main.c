#include <cglm/cglm.h>
#include <stdio.h>
#include <utils.h>
#include <render/render.h>
#include <window/window.h>

int main(MAYBE_UNUSED int argc, MAYBE_UNUSED char **argv)
{
    Render render = {};
    Window curr_window = {};
    window_engine_init();
    window_init(&curr_window);

    render_engine_init();
    render_init(&render);
    while (!window_should_close(&curr_window))
    {
        window_update(&curr_window);
    }

    render_deinit(&render);
    render_engine_deinit();
    window_deinit(&curr_window);
    window_engine_deinit();
    return 0;
}
