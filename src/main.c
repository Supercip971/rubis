#include <cglm/cglm.h>
#include <render/render.h>
#include <stdio.h>
#include <utils.h>
#include <render/camera/camera.h>
#include <window/window.h>

int main(MAYBE_UNUSED int argc, MAYBE_UNUSED char **argv)
{
    Render render = {};

    Window curr_window = {};
    Camera cam = {};

    Scene scene = {};

    scene_init(&scene);

    scene_push_circle(&scene, vec3$(0,0,-1), 1, scene_push_lambertian(&scene, vec3$(1,0,0)));



    window_engine_init();
    window_init(&curr_window);

    render_engine_init(&curr_window, &scene);
    render_init(&render);

    camera_init(&cam, (void*)window_raw_handle(&curr_window));
    while (!window_should_close(&curr_window))
    {

        window_update(&curr_window);
        camera_update(&cam, (void*)window_raw_handle(&curr_window));
        render_engine_update_cam(&render, &cam);
        render_engine_frame(&render);
    }

    render_deinit(&render);
    render_engine_deinit();
    window_deinit(&curr_window);
    window_engine_deinit();
    return 0;
}
