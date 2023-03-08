#include "config.h"

Config default_cfg ={
     .camera_controllable = false,
        .window_width = WINDOW_WIDTH,
        .window_height = WINDOW_HEIGHT,
};
Config get_config()
{
    return default_cfg;
}

void set_config(Config cfg)
{
    default_cfg = cfg;
}
