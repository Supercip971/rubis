#include "config.h"

Config default_cfg = {
    .camera_controllable = false,
    .window_width = WINDOW_WIDTH,
    .window_height = WINDOW_HEIGHT,
    .show_ui = true,
    .rays_bounce = 4,
    .show_raster = false,
};
Config get_config()
{
    return default_cfg;
}

void set_config(Config cfg)
{
    default_cfg = cfg;
}
