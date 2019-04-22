#ifndef KRENDER_H
#define KRENDER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "kthread.h"

extern GLuint tex_akko;
extern GLuint tex_ritsu;

struct krender_args {
    GLFWwindow *window;
};

extern void *krender(void *arg);

#endif
