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

struct kobj {
    struct kdraw *kdraw;
    struct vec3 vel;
};

extern void game(GLFWwindow *window) {
    srand(time(NULL));

    GLuint tex_akko = ktex_load("res/tga/akko.tga");
    GLuint tex_ritsu = ktex_load("res/tga/ritsu128.tga");

    kdraw_init(window);

    struct kobj enemies[1000] = { 0 };
    for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
        enemies[i].kdraw = kdraw_make_quad(tex_ritsu, KDRAW_PROJ_PERSP);
        enemies[i].kdraw->pos = (struct vec3) {
            randf(-200, 200),
            randf(-200, 200),
            randf(-400, 0),
        };
        //enemies[i].vel.x = randf(-0.001, 0.001);
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

    // Loop
    struct ktiming_timer phys_timer;
    ktiming_starttimer(&phys_timer);

    uint64_t frame = 1;
    while (!glfwWindowShouldClose(window)) {
        struct ktiming_timer frame_timer;
        struct ktiming_timer render_timer;
        struct ktiming_timer wait_timer;

        ktiming_frame_started();
        ktiming_starttimer(&frame_timer);

        // Input
        glfwPollEvents();
        player.vel = (struct vec3){ 0, 0, 0 };
        if (kinput.up)
            player.vel.y = 3;
        if (kinput.down)
            player.vel.y = -3;
        if (kinput.left)
            player.vel.x = -3;
        if (kinput.right)
            player.vel.x = 3;

        // Physics
        double phys_time = ktiming_endtimer(&phys_timer);
        for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
            enemies[i].kdraw->pos.x += enemies[i].vel.x * phys_time;
            enemies[i].kdraw->pos.y += enemies[i].vel.y * phys_time;
            enemies[i].kdraw->pos.z += enemies[i].vel.z * phys_time;
            if (enemies[i].kdraw->pos.z > 10)
                enemies[i].kdraw->pos.z = -200;
        }
        player.kdraw->pos.x += player.vel.x * phys_time;
        player.kdraw->pos.y += player.vel.y * phys_time;
        player.kdraw->pos.z += player.vel.z * phys_time;

        // Render
        ktiming_starttimer(&render_timer);
        kdraw_draw(window);
        double render_time = ktiming_endtimer(&render_timer);

        // Don't run too fast
        ktiming_starttimer(&wait_timer);
        ktiming_wait();
        double wait_time = ktiming_endtimer(&wait_timer);

        double frame_time = ktiming_endtimer(&frame_timer);
        kprint("frame %05" PRIu64 ": (t)%08.04fms (r)%08.04fms (w)%08.04fms",
                frame, frame_time * 1000, render_time * 1000, wait_time * 1000);
        frame++;
    }
}
