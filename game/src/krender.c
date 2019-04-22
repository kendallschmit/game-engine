#include "krender.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "kutil.h"
#include "game.h"
#include "kengine.h"
#include "ktiming.h"
#include "kthread.h"
#include "vectors.h"
#include "kdraw.h"
#include "ktex.h"
#include "shad.h"

#define BILLION 1000000000
#define MILLION 1000000
#define SCALE MILLION

GLuint tex_akko;
GLuint tex_ritsu;

extern void *krender(void *arg)
{
    struct kthread *kthread = (struct kthread *)arg;
    struct krender_args *args = (struct krender_args *)kthread->arg;

    // Set up OpenGL context
    glfwMakeContextCurrent(args->window); // Get context for window
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress); // Load functions
    glfwSwapInterval(1); // Vsync

    shad_init(); // Init shaders
    kdraw_init(args->window);

    tex_akko = ktex_load("res/tga/akko.tga");
    tex_ritsu = ktex_load("res/tga/ritsu128.tga");

    kprint("Signalling init");
    kthread_signal_init(kthread); // Semaphore successful init
    while (!kthread->terminated) {
        // Track when this frame started
        ktiming_frame_started();
        //struct timespec ts = { 0, 8 * 1000000 };
        //nanosleep(&ts, NULL);

        // Calculate kdraw values
        kthread_lock(kthread);
        for (int i = 0; i < nkobjects; i++) {
            struct kobj *e = &kobjects[i];
            e->kdraw->pos.x = (GLfloat)e->pos.x / SCALE;
            e->kdraw->pos.y = (GLfloat)e->pos.y / SCALE;
            e->kdraw->pos.z = (GLfloat)e->pos.z / SCALE;
        }
        kthread_unlock(kthread);

        // Draw and swap
        kdraw_draw();
        struct ktiming_timer swap_timer;
        ktiming_starttimer(&swap_timer);
        glfwSwapBuffers(args->window);
        uint64_t swap_time = ktiming_endtimer(&swap_timer);
        kprint("buffer swap time %08.04fms", (double)swap_time / 1000000);

        // Don't run too fast
        ktiming_wait();
    }
    // Deinit shaders
    shad_deinit();

    return NULL;
}
