#include "engine.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "render_thread.h"
#include "kge_util.h"
#include "kge_timer.h"
#include "kge_thread.h"
#include "vectors.h"
#include "shader.h"
#include "texture.h"
#include "vao.h"
#include "draw.h"

struct object_group background_objs = { 0 };
struct object_group foreground_objs = { 0 };

GLuint tex_akko;
GLuint tex_ritsu;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void engine_thread_fn(GLFWwindow *window, struct kge_thread *render_thread)
{
    // Set up game
    srand(time(NULL));

    kge_thread_lock(render_thread); // Lock render thread during init

    struct timespec init_time;
    kge_timer_now(&init_time);
    background_objs.prev_tick_time = init_time;
    foreground_objs.prev_tick_time = init_time;

    // Only orthographic object is player for now
    kprint("Set up player");
    struct kge_obj *player = &foreground_objs.objs[0];
    player->draw = &foreground_objs.draws[0];
    *player->draw = (struct draw){
        .vao = vaos[VAO_QUAD],
        .tex = tex_akko,
    };
    foreground_objs.count = 1;

    // Ritsus are all perspective
    kprint("Set up ritsus");
    GLuint nritsus = 10000;
    for (GLuint i = 0; i < nritsus; i++) {
        struct kge_obj *o = &background_objs.objs[i];
        o->draw = &background_objs.draws[i];
        *o->draw = (struct draw){
            .vao = vaos[VAO_QUAD],
            .tex = tex_ritsu,
        };
        o->pos = (struct vec3i) {
            randi(-1000 * SCALE, 1000 * SCALE),
            randi(-1000 * SCALE, 1000 * SCALE),
            randi(-1000 * SCALE, 10 * SCALE),
        };
        o->vel.z = 80;
        //o->vel.y = 80;
    }
    background_objs.count = nritsus;
    kge_thread_unlock(render_thread); // Render thread is safe to go

    // Loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        glfwPollEvents();
        kge_thread_lock(render_thread);

        // Apply velocity to player
        player->vel = (struct vec3i){ 0, 0, 0 };
        if (input.up)
            player->vel.y = 3;
        if (input.down)
            player->vel.y = -3;
        if (input.left)
            player->vel.x = -3;
        if (input.right)
            player->vel.x = 3;

        // Physics update
        struct timespec now;
        kge_timer_now(&now);
        physics_update(&foreground_objs,
                kge_timer_nanos_diff(&now, &foreground_objs.prev_tick_time));
        foreground_objs.prev_tick_time = now;

        kge_thread_unlock(render_thread);

        struct timespec ts = { 0, 1 * (NANOS / MILLIS) };
        nanosleep(&ts, NULL);
    }
}

extern int engine_go()
{
    // Subscribe to GLFW errors
    glfwSetErrorCallback(error_callback);
    // Get context with GLFW
    if (!glfwInit())
        return -1;
    // OpenGL 4.1, no backward/forward compatibility
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // Make the window
    GLFWwindow *window = glfwCreateWindow(640, 480, "Your Mom", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    // Register key-press callback
    glfwSetKeyCallback(window, input_key_callback);

    // Start render thread
    struct render_thread_args render_thread_args = { .window = window };
    struct kge_thread render_thread = { .arg = &render_thread_args };
    if (kge_thread_start(&render_thread, "/renderer", render_thread_fn) != 0) {
        kprint("Unable to start render thread");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    kprint("Waiting for render thread to initialize");
    kge_thread_wait_for_init(&render_thread);
    kprint("Done waiting");

    engine_thread_fn(window, &render_thread);

    kge_thread_end(&render_thread);
    kprint("Render thread ended");

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

extern void physics_update(struct object_group *group, uint64_t nanos)
{
    uint64_t deltatime = nanos / (NANOS / SCALE);
    // Physics (all integers)
    for (GLuint i = 0; i < group->count; i++) {
        struct kge_obj *o = &group->objs[i];
        o->pos.x += o->vel.x * deltatime;
        o->pos.y += o->vel.y * deltatime;
        o->pos.z += o->vel.z * deltatime;

        while (o->pos.z > 10 * SCALE) {
            o->pos.z -= 1010 * SCALE;
        }
    }
}
