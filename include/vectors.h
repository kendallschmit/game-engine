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

#endif
