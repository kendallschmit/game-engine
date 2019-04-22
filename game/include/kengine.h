#ifndef KENDGINE_H
#define KENDGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "vectors.h"

struct kobj {
    struct vec3i pos;
    struct vec3i vel;
    struct kdraw *kdraw;
};

extern size_t nkobjects;
extern struct kobj kobjects[100000];

#endif
