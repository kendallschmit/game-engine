#include "game.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "kutil.h"
#include "ktiming.h"
#include "krender.h"
#include "kthread.h"
#include "kengine.h"
#include "kinput.h"
#include "vectors.h"
#include "kdraw.h"
#include "ktex.h"
#include "shad.h"

GLfloat randf(GLfloat minimum, GLfloat maximum) {
    GLfloat r = (GLfloat)rand() / RAND_MAX;
    r *= maximum - minimum;
    return r += minimum;
}

GLint randi(GLfloat minimum, GLfloat maximum) {
    GLfloat r = (GLfloat)rand() / RAND_MAX;
    r *= maximum - minimum;
    return r += minimum;
}

#define BILLION 1000000000
#define MILLION 1000000
#define SCALE MILLION

extern void game(GLFWwindow *window)
{
    // Start renderer
    struct krender_args krender_args = { .window = window };
    struct kthread render_thread = { .arg = &krender_args };
    if (kthread_start(&render_thread, "/renderer", krender) != 0) {
        kprint("Unable to start renderer");
        return;
    }
    kprint("Waiting for render thread to initialize");
    kthread_wait_for_init(&render_thread);
    kprint("Done waiting");

    // Set up game
    srand(time(NULL));
    struct kobj *player = &kobjects[0];
    player->kdraw = kdraw_make_quad(tex_akko, KDRAW_PROJ_ORTHO);
    for (int i = 1; i < 10000; i++) {
        kobjects[i].kdraw = kdraw_make_quad(tex_ritsu, KDRAW_PROJ_PERSP);
        kobjects[i].pos = (struct vec3i) {
            randi(-400 * SCALE, 400 * SCALE),
            randi(-400 * SCALE, 400 * SCALE),
            randi(-400 * SCALE, 10 * SCALE),
        };
        //kobjects[i].vel.z = 20;
        kobjects[i].vel.y = 80;
    }
    nkobjects = 10000;

    // Loop
    struct ktiming_timer input_timer;
    ktiming_starttimer(&input_timer);

    while (!glfwWindowShouldClose(window)) {
        kthread_lock(&render_thread);

        // Input
        glfwPollEvents();
        player->vel = (struct vec3i){ 0, 0, 0 };
        if (kinput.up)
            player->vel.y = 3;
        if (kinput.down)
            player->vel.y = -3;
        if (kinput.left)
            player->vel.x = -3;
        if (kinput.right)
            player->vel.x = 3;

        // Physics (all integers)
        uint64_t phys_time = ktiming_endtimer(&input_timer) / (BILLION / SCALE);
        for (int i = 0; i < nkobjects; i++) {
            kobjects[i].pos.x += kobjects[i].vel.x * phys_time;
            kobjects[i].pos.y += kobjects[i].vel.y * phys_time;
            kobjects[i].pos.z += kobjects[i].vel.z * phys_time;
            if (kobjects[i].pos.z > 10 * SCALE)
                kobjects[i].pos.z -= 410 * SCALE;
            if (kobjects[i].pos.y > 200 * SCALE)
                kobjects[i].pos.y -= 400 * SCALE;
        }
        kthread_unlock(&render_thread);

        //kprint("input time %08.04fms", (double)phys_time / 1000);
        struct timespec ts = { 0, 1 * 1000000 };
        nanosleep(&ts, NULL);
    }
    kthread_end(&render_thread);
    kprint("Render thread ended");
}
