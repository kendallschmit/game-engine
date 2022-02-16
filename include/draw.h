#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>

#include <glad/glad.h>

#include "vectors.h"

struct draw {
    struct vec3 pos;
};

#define PROJECTION_ORTHOGRAPHIC 0
#define PROJECTION_PERSPECTIVE 1

extern GLuint draw_model_quad;
extern GLuint draw_model_cube;

void draw_init(GLfloat view_distance, GLfloat fov_rad, GLfloat near_clip,
        GLfloat far_clip, GLfloat ortho_width, GLfloat ortho_depth);

void draw_set_dimensions(GLuint w, GLuint h);
void draw_clear(void);

void draw_look_at(struct vec3 origin, struct vec3 target, struct vec3 up);

// Models
GLuint draw_add_model(GLfloat *verts, GLint size, GLenum mode);
void draw_models(GLuint model, GLuint tex, struct draw *draws, GLuint ndraws,
        GLuint projection);

// Lines
void draw_lines(struct vec2 position, struct vec2 scale, GLfloat width,
        struct vec3 color, GLfloat *values, GLuint nvalues);

#endif
