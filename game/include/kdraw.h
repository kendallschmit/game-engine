#ifndef KDRAW_H
#define KDRAW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "vectors.h"

#define VIEW_DIST 5.0

#define FOV_RAD 1.5
#define NEAR_CLIP 0.1
#define FAR_CLIP 400.0

#define ORTHO_WIDTH 4.5
#define ORTHO_DEPTH 400.0

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
extern void kdraw_set_dimensions(int w, int h);
extern void kdraw_draw();
extern struct kdraw *kdraw_make_quad(GLuint tex, int proj);

#endif
