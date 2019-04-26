#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <GLFW/glfw3.h>

extern void input_key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods);

struct input_set {
    bool up;
    bool down;
    bool left;
    bool right;
};

extern struct input_set input;

#endif
