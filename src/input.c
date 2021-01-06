#include "input.h"

#include <stdio.h>

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
        int mods);
static void window_close_callback(GLFWwindow *window);

struct input_set input = { 0 };

void input_init(GLFWwindow *window)
{
    // Register key-press callback
    glfwSetKeyCallback(window, key_callback);
    // Register window close callback (click on X, alt+F4, etc.)
    glfwSetWindowCloseCallback(window, window_close_callback);
}

void input_poll(void)
{
    glfwPollEvents();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
        int mods)
{
    bool *val;
    switch (key) {
        case GLFW_KEY_UP:
            val = &input.up;
            break;
        case GLFW_KEY_DOWN:
            val = &input.down;
            break;
        case GLFW_KEY_LEFT:
            val = &input.left;
            break;
        case GLFW_KEY_RIGHT:
            val = &input.right;
            break;
        case GLFW_KEY_ESCAPE:
            val = &input.quit;
            break;
        default:
            return;
    }
    *val = action != GLFW_RELEASE;
}

static void window_close_callback(GLFWwindow *window)
{
    input.quit = true;
}
