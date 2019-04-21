#ifndef KDRAW_H
#define KDRAW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "vectors.h"

#define VIEW_DIST 10

#define FOV_RAD 1.7
#define NEAR_CLIP 0.5
#define FAR_CLIP 200

#define ORTHO_MIN 5

#define KDRAW_PROJ_START 0
#define KDRAW_PROJ_ORTHO 0
#define KDRAW_PROJ_PERSP 1
#define KDRAW_PROJ_MAX 2

struct kdraw {
    GLuint vao;
    GLuint tex;
    struct vec3 pos;
};

extern void kdraw_init(GLFWwindow *window);
extern void kdraw_window_adapt(GLFWwindow *window);
extern void kdraw_draw(GLFWwindow *window);
extern struct kdraw *kdraw_make_quad(size_t tex, int proj);
extern size_t kdraw_add_tex(GLfloat *pix);

#endif
