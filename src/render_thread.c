#include "render_thread.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine.h"
#include "kge_util.h"
#include "kge_timer.h"
#include "kge_thread.h"
#include "vectors.h"
#include "shader.h"
#include "texture.h"
#include "vao.h"
#include "draw.h"

#define TARGET_FPS 60
#define LATE_RENDER_SAFETY_NANOS (3 * NANOS / MILLIS)
#define BAD_AVERAGE_RATIO 0.998

static void render_update(struct object_group *group)
{
    for (GLuint i = 0; i < group->count; i++) {
        struct kge_obj *o = &group->objs[i];
        o->draw->pos.x = (GLfloat)o->pos.x / SCALE;
        o->draw->pos.y = (GLfloat)o->pos.y / SCALE;
        o->draw->pos.z = (GLfloat)o->pos.z / SCALE;
    }
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_size_callback(GLFWwindow *window, int x, int y)
{
    draw_set_dimensions((GLfloat)x, (GLfloat)y);
}

extern void *render_thread_fn(void *thread_arg)
{
    // Get args
    struct kge_thread *thread = (struct kge_thread *)thread_arg;
    struct render_thread_args *args = thread->arg;

    // Set up OpenGL context
    glfwMakeContextCurrent(args->window); // Get context for window
    gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress); // Load functions
    glfwSwapInterval(1); // Vsync

    shader_init(); // Init shaders
    vaos_init(); // Init shaders
    draw_init(5, 1.5, 0.1, 1000, 6, 1000); // Init draw

    // Window dimensions
    int winx, winy;
    glfwGetWindowSize(args->window, &winx, &winy);
    draw_set_dimensions((GLfloat)winx, (GLfloat)winy);
    glfwSetWindowSizeCallback(args->window, window_size_callback);

    tex_akko = texture_load("res/tga/akko.tga");
    tex_ritsu = texture_load("res/tga/ritsu128.tga");

    // Done with initialization
    kprint("Signalling init");
    kge_thread_signal_init(thread);

    uint64_t bad_average_render_time = NANOS / TARGET_FPS;
    // Main render loop
    while (!thread->terminated) {
        uint64_t sleep_ns = (NANOS / TARGET_FPS) - LATE_RENDER_SAFETY_NANOS;
        if (sleep_ns > bad_average_render_time) sleep_ns -= bad_average_render_time;
        else sleep_ns = 0;
        //kprint("%08.04fms(av. worst), %08.04fms(sleep),", (double)bad_average_render_time / NANOS * MILLIS, (double)sleep_ns / NANOS * MILLIS)
        struct timespec ts = { 0, sleep_ns };
        nanosleep(&ts, NULL);

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
                | GL_STENCIL_BUFFER_BIT);
        draw_list(background_objs.draws, background_objs.count,
                PROJECTION_PERSPECTIVE, true, true);
        draw_list(foreground_objs.draws, foreground_objs.count,
                PROJECTION_ORTHOGRAPHIC, true, true);

        struct timespec renderend;
        kge_timer_now(&renderend);
        uint64_t render_time = kge_timer_nanos_diff(&renderend, &renderstart);
        if (render_time > bad_average_render_time)
            bad_average_render_time = render_time * BAD_AVERAGE_RATIO + bad_average_render_time * (1 - BAD_AVERAGE_RATIO);
        else
            bad_average_render_time = render_time * (1 - BAD_AVERAGE_RATIO) + bad_average_render_time * BAD_AVERAGE_RATIO;

        glfwSwapBuffers(args->window);
    }
    // Deinit engine stuff
    shader_deinit();
    vaos_deinit();

    return 0;
}

