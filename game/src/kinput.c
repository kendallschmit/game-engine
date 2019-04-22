#include "kinput.h"

#include "kutil.h"

struct kinput_set kinput = { 0 };

extern void kinput_key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods)
{
    bool *val;
    switch (key) {
        case GLFW_KEY_UP:
            val = &kinput.up;
            break;
        case GLFW_KEY_DOWN:
            val = &kinput.down;
            break;
        case GLFW_KEY_LEFT:
            val = &kinput.left;
            break;
        case GLFW_KEY_RIGHT:
            val = &kinput.right;
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
