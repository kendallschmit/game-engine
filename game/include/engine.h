#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "vectors.h"
#include "draw.h"

struct kge_obj {
    struct vec3i pos;
    struct vec3i vel;
    struct draw *draw;
};

extern int engine_go();

#endif
