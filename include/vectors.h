#ifndef VECTORS_H
#define VECTORS_H

#include <glad/glad.h>

struct vec2 {
    GLfloat x, y;
};
struct vec2i {
    GLint x, y;
};

struct vec3 {
    GLfloat x, y, z;
};

struct vec3i {
    GLint x, y, z;
};

GLfloat vec3_magnitude(struct vec3 v);
struct vec3 vec3_norm(struct vec3 v);
struct vec3 vec3_neg(struct vec3 v);
struct vec3 vec3_sum(struct vec3 a, struct vec3 b);
struct vec3 vec3_diff(struct vec3 a, struct vec3 b);
GLfloat vec3_dot(struct vec3 a, struct vec3 b);
struct vec3 vec3_cross(struct vec3 a, struct vec3 b);

#endif
