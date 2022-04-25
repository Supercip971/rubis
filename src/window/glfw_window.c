
#include <GLFW/glfw3.h>
#include <config.h>
#include <ds/vec.h>
#include <stdio.h>
#include <utils.h>
#include <window/window.h>

// FIXME: add a function for getting window safely
typedef struct
{
    Window self;
    GLFWwindow *window;
} WindowImpl;

vec_t(WindowImpl) windows;

int window_engine_init(void)
{
    glfwInit();
    vec_init(&windows);
    return 0;
}

int window_init(Window *self)
{
    WindowImpl impl = {};
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    self->window_id = windows.length;
    impl.window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "loading", NULL, NULL);

    vec_push(&windows, impl);
    return 0;
}

int window_deinit(Window *self)
{
    WindowImpl *impl = &windows.data[self->window_id];
    glfwDestroyWindow(impl->window);
    return 0;
}

int window_engine_deinit(void)
{
    vec_deinit(&windows);
    glfwTerminate();
    return 0;
}

/*
int window_width(Window *self)
{
    WindowImpl *impl = &windows.data[self->window_id];
    int width;
    int height;
    glfwGetWindowSize(impl->window, &width, &height);
    return width;
}

int window_height(Window *self)
{

    WindowImpl *impl = &windows.data[self->window_id];
    int width;
    int height;
    glfwGetWindowSize(impl->window, &width, &height);
    return height;
}
*/

int window_width(MAYBE_UNUSED Window *self)
{
    return WINDOW_WIDTH;
}

int window_height(MAYBE_UNUSED Window *self)
{

    return WINDOW_HEIGHT;
}

bool window_should_close(Window *self)
{
    WindowImpl *impl = &windows.data[self->window_id];

    return glfwWindowShouldClose(impl->window);
}

int window_update(MAYBE_UNUSED Window *self)
{
    glfwPollEvents();
    return 0;
}
