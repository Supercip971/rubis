#include <GLFW/glfw3.h>
#include <math.h>
#include <render/camera/camera.h>
#include <stdio.h>
#include <utils.h>
#include "math/mat4.h"
Camera *cam2;

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    (void)window;
    (void)xpos;
    (void)ypos;
    double x = xpos;
    double y = ypos;

    float dx = x - cam2->lastx;
    float dy = cam2->lasty - y;

    cam2->yaw += dx * 0.05f;
    cam2->pitch += dy * 0.05f;

    cam2->pitch = clamp(cam2->pitch, -89.0f, 89.0f);

    cam2->front.x = cosf(DEG2RAD(cam2->yaw)) * cosf(DEG2RAD(cam2->pitch));
    cam2->front.y = sinf(DEG2RAD(cam2->pitch));
    cam2->front.z = sinf(DEG2RAD(cam2->yaw)) * cosf(DEG2RAD(cam2->pitch));
    cam2->front = vec3_unit(cam2->front);
    cam2->lastx = x;
    cam2->lasty = y;
    (void)window;
}

Camera *ctx2;

void camera_key_fun(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        ctx2->denoise = !ctx2->denoise;
        printf("switch\n");
    }
    (void)mods;
    (void)scancode;
    (void)window;
}

void camera_scroll_mouse_fun(GLFWwindow *window, double xpos, double ypos)
{
    (void)xpos;
    ctx2->speed = clamp(ctx2->speed + ypos * 0.01f, 0.01f, 10.0f);
    (void)window;
}
void camera_init(Camera *cam, void *whandle, bool enable_control, Matrix4x4 *mod)
{
    GLFWwindow *window = whandle;

    if (enable_control)
    {
        double x;
        double y;
        glfwGetCursorPos(window, &x, &y);

        glfwSetCursorPosCallback(window, mouse_callback);
    }
    cam2 = cam;

    cam->pos = vec3$(0, 0, 0);

    matrix_apply_point(mod, &cam->pos);
    cam->front = vec3$(0, 0, -1.0f);
    matrix_apply_vector(mod, &cam->front);

    cam->up = vec3$(0, 1.0f, 0);
    matrix_apply_vector(mod, &cam->up);

    cam->yaw = 0;
    cam->pitch = 0;
    cam->denoise = true;
    cam->lastx = 0;
    cam->lasty = 0;
    cam->speed = 0.002f;
    cam->controllable = enable_control;
    ctx2 = cam;
    glfwSetKeyCallback(window, camera_key_fun);
    glfwSetScrollCallback(window, camera_scroll_mouse_fun);
}

// FIXME: use the window api
void camera_update(Camera *cam, void *whandle)
{
    cam2->front = vec3_unit(cam2->front);

    GLFWwindow *window = whandle;
    if (cam->controllable)
    {

        cam2 = cam;

        //const float cam_speed = 0.002f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            cam->pos = vec3_add(cam->pos, vec3_mul_val(cam->front, cam->speed));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            cam->pos = vec3_sub(cam->pos, vec3_mul_val(cam->front, cam->speed));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            cam->pos = vec3_add(cam->pos, vec3_mul_val(vec3_cross(cam->front, cam->up), cam->speed));
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            cam->pos = vec3_sub(cam->pos, vec3_mul_val(vec3_cross(cam->front, cam->up), cam->speed));
        }
     //   if(glfwGetKey(window, GLFW_MOUSE) == GLFW_PRESS)
     //   {
     //       cam->pos = vec3_add(cam->pos, vec3_mul_val(cam->up, cam_speed));
     //   }
    }
}
