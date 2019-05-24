#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "time.h"

#include "vectors.h"
#include "draw.h"

#define NANOS 1000000000l
#define MICROS 1000000l
#define MILLIS 1000l
#define SCALE MICROS

struct kge_obj {
    struct vec3i pos;
    struct vec3i vel;
    struct draw *draw;
};

#define OBJECTS_MAX 100000
struct object_group {
    struct draw draws[OBJECTS_MAX];
    struct kge_obj objs[OBJECTS_MAX];
    GLuint count;
    struct timespec prev_tick_time;
};

extern struct object_group background_objs;
extern struct object_group foreground_objs;

extern GLuint tex_akko;
extern GLuint tex_ritsu;

extern int engine_go();
extern void physics_update(struct object_group *group, uint64_t nanos);

#endif
