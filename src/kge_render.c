#include "kge_render.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "kge_util.h"
#include "vectors.h"
#include "shader.h"
#include "texture.h"
#include "vao.h"
#include "draw.h"

static void error_callback(int error, const char* description);
static void window_size_callback(GLFWwindow *glfw_window, int x, int y);

void kge_render_init(GLFWwindow *window)
{
    // Set up OpenGL context
    glfwMakeContextCurrent(window); // Get context for window
    gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress); // Load functions
    glfwSwapInterval(1); // Vsync

    glfwSetWindowSizeCallback(window, window_size_callback);
}

void kge_render_update_window_size(GLFWwindow *window)
{
    int winx, winy;
    glfwGetWindowSize(window, &winx, &winy);
    draw_set_dimensions((GLfloat)winx, (GLfloat)winy);
}

void kge_render_start(void)
{
    glClear(GL_COLOR_BUFFER_BIT
            | GL_DEPTH_BUFFER_BIT
            | GL_STENCIL_BUFFER_BIT);
}

void kge_render_finish(GLFWwindow *window)
{
    glfwSwapBuffers(window);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_size_callback(GLFWwindow *glfw_window, int x, int y)
{
    draw_set_dimensions((GLfloat)x, (GLfloat)y);
}
