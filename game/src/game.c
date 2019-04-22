#include "game.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "kutil.h"
#include "ktiming.h"
#include "kinput.h"
#include "vectors.h"
#include "kdraw.h"
#include "ktex.h"

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

struct vec3i {
    GLint x, y, z;
};

struct kobj {
    struct kdraw *kdraw;
    struct vec3i pos;
    struct vec3i vel;
};

extern void game(GLFWwindow *window) {
    srand(time(NULL));

    GLuint tex_akko = ktex_load("res/tga/akko.tga");
    GLuint tex_ritsu = ktex_load("res/tga/ritsu128.tga");

    kdraw_init(window);

    struct kobj enemies[1000] = { 0 };
    for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
        enemies[i].kdraw = kdraw_make_quad(tex_ritsu, KDRAW_PROJ_PERSP);
        enemies[i].pos = (struct vec3i) {
            randi(-200 * SCALE, 200 * SCALE),
            randi(-200 * SCALE, 200 * SCALE),
            randi(-400 * SCALE, 0 * SCALE),
        };
        enemies[i].vel.z = 20;
    }

    struct kobj player = { 0 };
    player.kdraw = kdraw_make_quad(tex_akko, KDRAW_PROJ_ORTHO);

    kprint("WGL_EXT_swap_control_tear supported: %s",
            glfwExtensionSupported("WGL_EXT_swap_control_tear")
            ? "true" : "false");
    kprint("GLX_EXT_swap_control_tear supported: %s",
            glfwExtensionSupported("GLX_EXT_swap_control_tear")
            ? "true" : "false");

    glfwSwapInterval(1); // Use vsync
    // Loop
    struct ktiming_timer phys_timer;
    ktiming_starttimer(&phys_timer);

    uint64_t frame = 1;
    while (!glfwWindowShouldClose(window)) {
        struct ktiming_timer frame_timer;
        struct ktiming_timer input_to_swap_timer;
        struct ktiming_timer render_timer;
        struct ktiming_timer wait_timer;

        ktiming_frame_started();
        ktiming_starttimer(&frame_timer);

        struct timespec ts = { 0, 9000000 };
        nanosleep(&ts, NULL);

        // Input
        ktiming_starttimer(&input_to_swap_timer);
        glfwPollEvents();
        player.vel = (struct vec3i){ 0, 0, 0 };
        if (kinput.up)
            player.vel.y = 3;
        if (kinput.down)
            player.vel.y = -3;
        if (kinput.left)
            player.vel.x = -3;
        if (kinput.right)
            player.vel.x = 3;

        // Physics (all integers)
        uint64_t phys_time = ktiming_endtimer(&phys_timer) / (BILLION / SCALE);
        for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
            enemies[i].pos.x += enemies[i].vel.x * phys_time;
            enemies[i].pos.y += enemies[i].vel.y * phys_time;
            enemies[i].pos.z += enemies[i].vel.z * phys_time;
            if (enemies[i].pos.z > 10 * SCALE)
                enemies[i].pos.z = -200 * SCALE;
        }
        player.pos.x += player.vel.x * phys_time;
        player.pos.y += player.vel.y * phys_time;
        player.pos.z += player.vel.z * phys_time;

        // Render
        for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
            struct kobj *e = &enemies[i];
            e->kdraw->pos.x = (GLfloat)e->pos.x / SCALE;
            e->kdraw->pos.y = (GLfloat)e->pos.y / SCALE;
            e->kdraw->pos.z = (GLfloat)e->pos.z / SCALE;
        }
        player.kdraw->pos.x = (GLfloat)player.pos.x / SCALE;
        player.kdraw->pos.y = (GLfloat)player.pos.y / SCALE;
        player.kdraw->pos.z = (GLfloat)player.pos.z / SCALE;
        ktiming_starttimer(&render_timer);
        kdraw_draw(window);
        double render_time = (double)ktiming_endtimer(&render_timer) / BILLION;
        glfwSwapBuffers(window);
        double input_to_swap_time =
            (double)ktiming_endtimer(&input_to_swap_timer) / BILLION;

        // Don't run too fast
        ktiming_starttimer(&wait_timer);
        ktiming_wait();
        double wait_time = (double)ktiming_endtimer(&wait_timer) / BILLION;

        double frame_time = (double)ktiming_endtimer(&frame_timer) / BILLION;
        kprint("frame %05" PRIu64 ": (t)%08.04fms (r)%08.04fms (w)%08.04fms"
                " (i/s)%08.04fms",
                frame, frame_time * 1000, render_time * 1000, wait_time * 1000,
                input_to_swap_time * 1000);
        frame++;
    }
}
