#pragma once

#include <math/vec3.h>
#include "math/mat4.h"

typedef struct
{
    Vec3 pos;
    Vec3 front;
    float speed;
    Vec3 up;

    float lastx;
    float lasty;
    float yaw;
    float pitch;
    float aperture;
    float focus_disc;
    bool controllable;
    bool denoise;
} Camera;

void camera_init(Camera *cam, void *whandle, Matrix4x4* mod);

void camera_update(Camera *cam, void *whandle);
