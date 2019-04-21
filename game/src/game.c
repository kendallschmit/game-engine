#include "game.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "kutil.h"
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

void game(GLFWwindow *window) {
    srand(time(NULL));

    GLuint tex_akko = ktex_load("res/tga/akko.tga");
    GLuint tex_ritsu = ktex_load("res/tga/ritsu.tga");

    kdraw_init(window);

    struct kobj enemies[10] = { 0 };
    for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
        enemies[i].kdraw = kdraw_make_quad(tex_akko, KDRAW_PROJ_PERSP);
        enemies[i].kdraw->pos = (struct vec3) {
            randf(-1, 1),
            randf(-1, 1),
            randf(-30, 1),
        };
        enemies[i].vel.x = randf(-0.001, 0.001);
    }

    struct kobj player = { 0 };

    // Loop
    size_t frames;
    while (!glfwWindowShouldClose(window)) {
        for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
            enemies[i].kdraw->pos.x += enemies[i].vel.x;
        }
        kdraw_draw(window);
    }
}
