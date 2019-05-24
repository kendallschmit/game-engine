#ifndef RENDER_THREAD_H
#define RENDER_THREAD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "vectors.h"
#include "draw.h"

struct render_thread_args
{
    GLFWwindow *window;
};

extern void *render_thread_fn(void *thread_arg);

#endif
