#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <GLFW/glfw3.h>

struct input_set {
    bool up;
    bool down;
    bool left;
    bool right;

    bool quit;
};

extern struct input_set input;

void input_init(GLFWwindow *window);

void input_poll(void);

#endif
