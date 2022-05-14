#pragma once 

#include <math/vec3.h>

typedef struct 
{
    Vec3 pos;
    Vec3 front;
    Vec3 up;

    float lastx;
    float lasty;
    float yaw;
    float pitch;
    float aperture;
    float focus_disc;
    bool controllable;
} Camera;


void camera_init(Camera* cam, void* whandle, bool enable_control);

void camera_update(Camera* cam, void* whandle);
