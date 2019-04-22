#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>

#include <glad/glad.h>

#include "vectors.h"

#define PROJECTION_ORTHOGRAPHIC 0
#define PROJECTION_PERSPECTIVE 1

struct draw {
    GLuint vao;
    GLuint tex;
    struct vec3 pos;
};

extern void draw_init(GLfloat view_distance, GLfloat fov_rad, GLfloat near_clip,
        GLfloat far_clip, GLfloat ortho_width, GLfloat ortho_depth);
extern void draw_set_dimensions(GLuint w, GLuint h);

extern void draw_list(struct draw *draws, GLuint ndraws, GLuint projection,
        bool same_vao, bool same_tex);

#endif
