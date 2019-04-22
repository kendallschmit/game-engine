#include "game.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "kutil.h"
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
        enemies[i].vel.z = 0.05;
    }

    struct kobj player = { 0 };
    player.kdraw = kdraw_make_quad(tex_akko, KDRAW_PROJ_ORTHO);

    // Loop
    uint64_t frames = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (kinput.up)
            player.kdraw->pos.y += 0.02;
        if (kinput.down)
            player.kdraw->pos.y -= 0.02;
        if (kinput.left)
            player.kdraw->pos.x -= 0.02;
        if (kinput.right)
            player.kdraw->pos.x += 0.02;
        for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
            enemies[i].kdraw->pos.x += enemies[i].vel.x;
            enemies[i].kdraw->pos.y += enemies[i].vel.y;
            enemies[i].kdraw->pos.z += enemies[i].vel.z;
            if (enemies[i].kdraw->pos.z > 10)
                enemies[i].kdraw->pos.z = -200;
        }
        kdraw_draw(window);
        kprint("frames %" PRIu64, frames);
        frames++;
    }
}
