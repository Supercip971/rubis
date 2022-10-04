#include <config.h>
#include <gltf/gltf.h>
#include <render/camera/camera.h>
#include <render/render.h>
#include <stdio.h>
#include <utils.h>
#include <utils/file.h>
#include <window/window.h>

#define randomf() (((float)random()) / (float)RAND_MAX)
int main(MAYBE_UNUSED int argc, MAYBE_UNUSED char **argv)
{
    Render render = {};

    Window curr_window = {};
    Camera cam = {};

    Scene scene = {};

    scene_init(&scene);
    Buffer skybuf = read_file("obj/skymap.png");
    scene.skymap = image_load(skybuf.buffer, skybuf.len);

    printf("loading scene...\n");
    // Buffer fbuf = read_file("obj/light-spheres.glb");
    Buffer fbuf = read_file("obj/sponza.glb");

    // Buffer fbuf = read_file("obj/bedroom.glb");

    if (parse_gltf(fbuf.buffer, &scene) == false)
    {
        return -1;
    }
    printf("loaded scene %i \n", scene.meshes.length);

    window_engine_init();

    window_init(&curr_window);

    render_engine_init(&curr_window, &scene);
    render_init(&render);

    camera_init(&cam, (void *)window_raw_handle(&curr_window), CAMERA_CONTROLLABLE, &scene.camera_transform);
    cam.aperture = 8.0;
    cam.focus_disc = (2.0f);
   // cam.pos = vec3$(0, 2, 1);
   // cam.front = vec3_unit(vec3_sub(vec3$(0, 2, 0), cam.pos));
    while (!window_should_close(&curr_window))
    {

        window_update(&curr_window);
        camera_update(&cam, (void *)window_raw_handle(&curr_window));
        render_engine_update_cam(&render, &cam);
        render_engine_frame(&render);
    }

    render_deinit(&render);
    render_engine_deinit();
    window_deinit(&curr_window);
    window_engine_deinit();
    return 0;
}
