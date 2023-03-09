#include <GLFW/glfw3.h>
#include <config.h>
#include <ds/vec.h>
#include <render/render.h>
#include <stdio.h>
#include <utils.h>
#include <window/window.h>

// FIXME: add a function for getting window safely
typedef struct
{
    Window self;
    GLFWwindow *window;
    bool control;
} WindowImpl;

vec_t(WindowImpl) windows = {};

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

    impl.self = *self;
    impl.control = !get_config().camera_controllable;
    printf("window: %lx\n", (uintptr_t)impl.window);

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

int window_surface_init(Window *self, Render *render)
{
    render_surface_init(render, (uintptr_t)windows.data[self->window_id].window);

    return 0;
}

int window_surface_deinit(Window *self, Render *render)
{
    render_surface_deinit(render, (uintptr_t)windows.data[self->window_id].window);

    return 0;
}

uintptr_t window_raw_handle(Window *self)
{
    return (uintptr_t)windows.data[self->window_id].window;
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

bool pressed_h = false;
bool pressed_c = false;
int window_update(MAYBE_UNUSED Window *self)
{
    WindowImpl *impl = &windows.data[self->window_id];

    bool nc = get_config().camera_controllable;

    if (nc != impl->control)
    {
        if (get_config().camera_controllable)
        {
            glfwSetInputMode(impl->window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            nc = true;
        }
        else
        {
            glfwSetInputMode(impl->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            nc = false;
        }
        impl->control = nc;
    }

    glfwPollEvents();

    if (glfwGetKey(impl->window, GLFW_KEY_H) == GLFW_PRESS)
    {
        if (!pressed_h)
        {
            Config c = get_config();
            c.show_ui = !c.show_ui;
            set_config(c);
        }
        pressed_h = true;
    }
    else
    {
        pressed_h = false;
    }
    if (glfwGetKey(impl->window, GLFW_KEY_C) == GLFW_PRESS)
    {
        if (!pressed_c)
        {
            Config c = get_config();
            c.camera_controllable = !c.camera_controllable;
            set_config(c);
        }
        pressed_c = true;
    }
    else
    {
        pressed_c = false;
    }
    return 0;
}
