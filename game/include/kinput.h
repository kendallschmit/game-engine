#ifndef KINPUT_H
#define KINPUT_H

#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct kinput_set {
    bool up;
    bool down;
    bool left;
    bool right;
};

extern struct kinput_set kinput;

extern void kinput_key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods);

#endif
