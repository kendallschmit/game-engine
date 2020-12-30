#ifndef RENDER_THREAD_H
#define RENDER_THREAD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "vectors.h"
#include "draw.h"

void kge_render_init(GLFWwindow *window);

void kge_render_update_window_size(GLFWwindow *window);

void kge_render_start(void);
void kge_render_finish(GLFWwindow *window);

#endif
