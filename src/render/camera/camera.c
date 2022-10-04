#include <GLFW/glfw3.h>
#include <math.h>
#include <render/camera/camera.h>
#include <stdio.h>
#include <utils.h>
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
void camera_init(Camera *cam, void *whandle, bool enable_control)
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
    cam->pos = vec3$(0, 0, 1.0);

    cam->front = vec3$(0, 0, -1.0f);

    cam->up = vec3$(0, 1.0f, 0);
    cam->yaw = 0;
    cam->pitch = 0;
    cam->denoise = true;
    cam->lastx = 0;
    cam->lasty = 0;
    cam->controllable = enable_control;
    ctx2 = cam;
    glfwSetKeyCallback(window, camera_key_fun);
}

// FIXME: use the window api
void camera_update(Camera *cam, void *whandle)
{
    cam2->front = vec3_unit(cam2->front);

    GLFWwindow *window = whandle;
    if (cam->controllable)
    {

        cam2 = cam;

        const float cam_speed = 0.02f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            cam->pos = vec3_add(cam->pos, vec3_mul_val(cam->front, cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            cam->pos = vec3_sub(cam->pos, vec3_mul_val(cam->front, cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            cam->pos = vec3_add(cam->pos, vec3_mul_val(vec3_cross(cam->front, cam->up), cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            cam->pos = vec3_sub(cam->pos, vec3_mul_val(vec3_cross(cam->front, cam->up), cam_speed));
        }
    }
}
