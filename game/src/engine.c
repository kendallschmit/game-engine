#include "engine.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "kge_util.h"
#include "kge_timer.h"
#include "kge_thread.h"
#include "vectors.h"
#include "shader.h"
#include "texture.h"
#include "vao.h"
#include "draw.h"

#define NANOS 1000000000
#define MICROS 1000000
#define MILLIS 1000
#define SCALE MICROS

// Dealing with objects
#define OBJECTS_MAX 100000
struct object_group {
    struct draw draws[OBJECTS_MAX];
    struct kge_obj objs[OBJECTS_MAX];
    GLuint count;
    struct timespec prev_tick_time;
};

struct object_group background_objs;
struct object_group foreground_objs;

static void physics_update(struct object_group *group, uint64_t nanos)
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

static void render_update(struct object_group *group)
{
    for (GLuint i = 0; i < group->count; i++) {
        struct kge_obj *o = &group->objs[i];
        o->draw->pos.x = (GLfloat)o->pos.x / SCALE;
        o->draw->pos.y = (GLfloat)o->pos.y / SCALE;
        o->draw->pos.z = (GLfloat)o->pos.z / SCALE;
    }
}

static GLuint tex_akko;
static GLuint tex_ritsu;

// Input/main thread stuff
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void main_thread(GLFWwindow *window, struct kge_thread *render_thread)
{
    // Set up game
    srand(time(NULL));

    kge_thread_lock(render_thread); // Lock render thread during init
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

// Render stuff
static void window_size_callback(GLFWwindow *window, int x, int y)
{
    draw_set_dimensions((GLfloat)x, (GLfloat)y);
}

struct render_thread_args
{
    GLFWwindow *window;
};

static void *render_thread_fn(void *thread_arg)
{
    // Get args
    struct kge_thread *thread = (struct kge_thread *)thread_arg;
    struct render_thread_args *args = thread->arg;

    // Set up OpenGL context
    glfwMakeContextCurrent(args->window); // Get context for window
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress); // Load functions
    glfwSwapInterval(1); // Vsync

    shader_init(); // Init shaders
    vaos_init(); // Init shaders
    draw_init(5, 1.5, 0.1, 1000, 6, 1000); // Init draw

    // Window dimensions
    glfwSetWindowSizeCallback(args->window, window_size_callback);
    draw_set_dimensions((GLfloat)100, (GLfloat)100); // TODO

    tex_akko = texture_load("res/tga/akko.tga");
    tex_ritsu = texture_load("res/tga/ritsu128.tga");

    // Done with initialization
    kprint("Signalling init");
    kge_thread_signal_init(thread);

    // Main render loop
    while (!thread->terminated) {
        //struct timespec ts = { 0, 8 * (NANOS / MILLIS) };
        //nanosleep(&ts, NULL);

        struct timespec renderstart;
        kge_timer_now(&renderstart);
        // Calculate object positions
        kge_thread_lock(thread);

        // Physics and timing
        struct timespec now;
        kge_timer_now(&now);
        physics_update(&background_objs,
                kge_timer_nanos_diff(&now, &background_objs.prev_tick_time));
        physics_update(&foreground_objs,
                kge_timer_nanos_diff(&now, &foreground_objs.prev_tick_time));
        background_objs.prev_tick_time = now;
        foreground_objs.prev_tick_time = now;

        // Render
        render_update(&background_objs);
        render_update(&foreground_objs);

        kge_thread_unlock(thread);

        // Draw and swap
        struct timespec drawstart;
        kge_timer_now(&drawstart);

        glClear(GL_COLOR_BUFFER_BIT
                | GL_DEPTH_BUFFER_BIT
                | GL_ACCUM_BUFFER_BIT
                | GL_STENCIL_BUFFER_BIT);
        draw_list(background_objs.draws, background_objs.count,
                PROJECTION_PERSPECTIVE, true, true);
        draw_list(foreground_objs.draws, foreground_objs.count,
                PROJECTION_ORTHOGRAPHIC, true, true);

        struct timespec renderend;
        kge_timer_now(&renderend);
        kprint("%08.04fms (total), %08.04fms(draw),",
                (double)kge_timer_nanos_diff(&renderend, &renderstart)
                / NANOS * 1000,
                (double)kge_timer_nanos_diff(&renderend, &drawstart)
                / NANOS * 1000);

        glfwSwapBuffers(args->window);
    }
    // Deinit engine stuff
    shader_deinit();
    vaos_deinit();

    return 0;
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

    main_thread(window, &render_thread);

    kge_thread_end(&render_thread);
    kprint("Render thread ended");

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}
