#include "game.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "kge.h"
#include "kge_timer.h"
#include "input.h"
#include "kge_util.h"
#include "vectors.h"
#include "shader.h"
#include "texture.h"
#include "draw.h"

#define NANOS 1000000000l
#define MICROS 1000000l
#define MILLIS 1000l
#define SCALE MICROS

#define MOVE_SPEED 40

struct obj {
    struct vec3i pos;
    struct vec3i vel;
    struct draw *draw;
};

#define OBJECTS_MAX 1000000
struct obj_group {
    GLuint tex;
    GLuint model;
    struct draw draws[OBJECTS_MAX];
    struct obj objs[OBJECTS_MAX];
    GLuint count;
};

static void error_callback(int error, const char* description);

static void game_loop();

static void update(uint64_t now);

static void physics_update(struct obj_group *group, uint64_t nanos);
static void update_draw_positions(struct obj_group *group);

int game_run(void)
{
    if (kge_init() == -1)
        return -1;

    game_loop();

    kge_deinit();

    return 0;
}

static void game_loop()
{
    static struct obj_group background_objs = { 0 };
    background_objs.model = draw_model_quad;
    background_objs.tex = texture_ritsu;
    static struct obj_group foreground_objs = { 0 };
    foreground_objs.model = draw_model_cube;
    foreground_objs.tex = texture_akko;

    // Only orthographic object is player for now
    kprint("Set up player");
    struct obj *player = &foreground_objs.objs[0];
    player->draw = &foreground_objs.draws[0];
    *player->draw = (struct draw){ 0 };
    foreground_objs.count = 1;

    // Ritsus are all perspective
    kprint("Set up ritsus");
    GLuint nritsus = 100000;
    for (GLuint i = 0; i < nritsus; i++) {
        struct obj *o = &background_objs.objs[i];
        o->draw = &background_objs.draws[i];
        *o->draw = (struct draw){ 0 };
        o->pos = (struct vec3i) {
            randi(-1000 * SCALE, 1000 * SCALE),
            randi(-1000 * SCALE, 1000 * SCALE),
            randi(-1000 * SCALE, 0 * SCALE),
        };
    }
    background_objs.count = nritsus;

    uint64_t draw_time = 0;
    uint64_t draw_count = 0;

    uint64_t physics_time = 0;
    uint64_t physics_count = 0;

    struct vec3 view_origin = { 0, 100, 100 };

    // Gameplay loop
    while (!input.quit) {
        // Show the frame (blocks until it is displayed on screen)
        uint64_t delta_time = kge_show_frame();

        uint64_t input_start = kge_timer_now();

        // TODO Make input poll threaded
        input_poll();

        uint64_t input_end = kge_timer_now();
        uint64_t physics_start = input_end;

        // Apply velocity to player
        player->vel = (struct vec3i){ 0, 0, 0 };
        if (input.up)
            player->vel.z = -MOVE_SPEED;
        if (input.down)
            player->vel.z = MOVE_SPEED;
        if (input.left)
            player->vel.x = -MOVE_SPEED;
        if (input.right)
            player->vel.x = MOVE_SPEED;

        // Calculate object motion
        physics_update(&background_objs, delta_time);
        physics_update(&foreground_objs, delta_time);

        GLfloat f = 6 * ((GLfloat)delta_time) / NANOS;
        if (f > 1)
            f = 1;

        view_origin.y = 0;
        view_origin = vec3_norm(vec3_diff(view_origin, player->draw->pos));
        view_origin = vec3_scale(view_origin, 5);
        view_origin = vec3_sum(view_origin, player->draw->pos);

        draw_look_at(view_origin,
                player->draw->pos,
                (struct vec3){ 0, 1, 0 });

        uint64_t physics_end = kge_timer_now();
        uint64_t draw_start = physics_end;

        // Draw objects
        draw_clear();
        draw_models(background_objs.model, background_objs.tex,
                background_objs.draws, background_objs.count,
                PROJECTION_PERSPECTIVE);
        draw_models(foreground_objs.model, foreground_objs.tex,
                foreground_objs.draws, foreground_objs.count,
                PROJECTION_PERSPECTIVE);

        uint64_t draw_end = kge_timer_now();

        draw_time += draw_end - draw_start;
        draw_count++;

        physics_time += physics_end - physics_start;
        physics_count++;
    }
    kprint("average draw time: %fms",
            (float)(draw_time / draw_count) / 1000000);
    kprint("average physics time: %fms",
            (float)(physics_time / physics_count) / 1000000);
}

static void physics_update(struct obj_group *group, uint64_t nanos)
{
    uint64_t deltatime = nanos / (NANOS / SCALE);
    // Physics (all integers)
    for (GLuint i = 0; i < group->count; i++) {
        struct obj *o = &group->objs[i];
        o->pos.x += o->vel.x * deltatime;
        o->pos.y += o->vel.y * deltatime;
        o->pos.z += o->vel.z * deltatime;

        o->draw->pos.x = (GLfloat)o->pos.x / SCALE;
        o->draw->pos.y = (GLfloat)o->pos.y / SCALE;
        o->draw->pos.z = (GLfloat)o->pos.z / SCALE;
    }
}
