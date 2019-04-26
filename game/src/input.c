#include "input.h"

struct input_set input = { 0 };

extern void input_key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
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
        default:
            return;
    }
    if (action == GLFW_PRESS) {
        *val = true;
    }
    else if (action == GLFW_RELEASE) {
        *val = false;
    }
}
