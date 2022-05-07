#include <render/camera/camera.h>
#include <math.h>
#include <utils.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
Camera* cam2;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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
void camera_init(Camera* cam, void* whandle)
{
    GLFWwindow* window = whandle;

    double x;
    double y;
    glfwGetCursorPos(window, &x, &y);

    glfwSetCursorPosCallback(window, mouse_callback);
    cam2 = cam;
    cam->pos = vec3$(0,0,1.0);

    cam->front = vec3$(0,0,-1.0f);

    cam->up = vec3$(0,1.0f,0);
    cam->yaw = 0;
    cam->pitch = 0;
    cam->lastx = 0;
    cam->lasty = 0;
}

// FIXME: use the window api
void camera_update(Camera* cam, void* whandle)
{
    GLFWwindow* window = whandle;
    cam2 = cam;

    const float cam_speed = 0.1f;

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cam->pos = vec3_add(cam->pos, vec3_mul_val(cam->front, cam_speed));
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cam->pos = vec3_sub(cam->pos, vec3_mul_val(cam->front, cam_speed));
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cam->pos = vec3_add(cam->pos, vec3_mul_val(vec3_cross(cam->front, cam->up), cam_speed));
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cam->pos = vec3_sub(cam->pos, vec3_mul_val(vec3_cross(cam->front, cam->up), cam_speed));
    }

}
