#include <config.h>
#include <gltf/gltf.h>
#include <render/camera/camera.h>
#include <render/render.h>
#include <stdio.h>
#include <utils.h>
#include <utils/file.h>
#include <window/window.h>
#include <arg-parse/arg-parse.h>
#include "obj/scene.h"

struct muarg_argument_config arg_list[] = {
    MUARG_HELP(),
    MUARG_BOOL("camera-control", 'c', "make the camera controllable", NULL),
    MUARG_STRING("model", 'm', "select the object to use for rendering", NULL),
};

struct muarg_header header = {
    .app_name = "feather",
    .usage = "feather -m {model}",
    .help_info = "You may need to download a .glb file",
    .version = "0.0.0",
    .argument_count = sizeof(arg_list) / sizeof(arg_list[0]),
    .argument_list = arg_list,
};

#define randomf() (((float)random()) / (float)RAND_MAX)
int main(MAYBE_UNUSED int argc, MAYBE_UNUSED char **argv)
{
    struct muarg_result res = muarg_eval(&header, argc, argv);
    if (res.has_error == MUARG_ERROR)
    {
        return res.has_error;
    }
    if (muarg_status_from_name(&res, "help")->is_called)
    {
        return 0;
    }
    struct muarg_argument_status *status = muarg_status_from_name(&res, "model");
    if (!status->is_called)
    {
        printf("invalid usage, please use a model path, see --help for more information\n");
        return -1;
    }

    Render render = {};

    Window curr_window = {};
    Camera cam = {};

    Scene scene = {};

    scene_init(&scene);
    Buffer skybuf = read_file("./meta/skymap.png");
    scene.skymap = image_load(skybuf.buffer, skybuf.len);

    printf("loading scene...\n");
    // Buffer fbuf = read_file("obj/light-spheres.glb");
    Buffer fbuf = read_file(status->input);

    // Buffer fbuf = read_file("obj/bedroom.glb");

    if (parse_gltf(fbuf.buffer, &scene) == false)
    {
        return -1;
    }
    printf("started resizing textures...\n");
    scene_resize_textures(&scene);
    printf("loaded scene %i \n", scene.meshes.length);

    window_engine_init();

    window_init(&curr_window);

    camera_init(&cam, (void *)window_raw_handle(&curr_window), &scene.camera_transform);
    render_engine_init(&curr_window, &scene);
    render_init(&render);

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
