#include "engine.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "kge_render.h"
#include "kge_util.h"
#include "kge_timer.h"
#include "vectors.h"
#include "shader.h"
#include "texture.h"
#include "vao.h"

static void error_callback(int error, const char* description);

static void engine_loop(GLFWwindow *window);

static void physics_update(struct object_group *group, uint64_t nanos);
static void update_draw_positions(struct object_group *group);

int engine_go(void)
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
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }
    // Register key-press callback
    glfwSetKeyCallback(window, input_key_callback);

    kge_render_init(window);
    shader_init(); // Init shaders
    vaos_init(); // Init vaos
    draw_init(5, 1.5, 0.1, 1000, 6, 1000); // Init draw

    kge_render_update_window_size(window);

    // Engine loop
    kprint("Enter engine loop");
    engine_loop(window);
    kprint("Exit engine loop");

    // Clean up kge_render
    vaos_deinit();
    shader_deinit();

    // Clean up GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void engine_loop(GLFWwindow *window)
{
    kprint("Load textures");
    // Load textures
    GLuint tex_akko = texture_load("res/tga/akko.tga");
    GLuint tex_ritsu = texture_load("res/tga/ritsu128.tga");
    kprint("Done loading textures");

    // Set up game
    srand(time(NULL));

    struct timespec init_time;
    kge_timer_now(&init_time);

    static struct object_group background_objs = { 0 };
    static struct object_group foreground_objs = { 0 };

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

    // Loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        glfwPollEvents();

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
        physics_update(&background_objs,
                kge_timer_nanos_diff(&now, &background_objs.prev_tick_time));
        background_objs.prev_tick_time = now;
        physics_update(&foreground_objs,
                kge_timer_nanos_diff(&now, &foreground_objs.prev_tick_time));
        foreground_objs.prev_tick_time = now;

        update_draw_positions(&background_objs);
        update_draw_positions(&foreground_objs);

        // Render
        kge_render_start();

        draw_list(background_objs.draws, background_objs.count,
                PROJECTION_PERSPECTIVE, true, true);
        draw_list(foreground_objs.draws, foreground_objs.count,
                PROJECTION_ORTHOGRAPHIC, true, true);

        kge_render_finish(window);
    }
}

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

static void update_draw_positions(struct object_group *group)
{
    for (GLuint i = 0; i < group->count; i++) {
        struct kge_obj *o = &group->objs[i];
        o->draw->pos.x = (GLfloat)o->pos.x / SCALE;
        o->draw->pos.y = (GLfloat)o->pos.y / SCALE;
        o->draw->pos.z = (GLfloat)o->pos.z / SCALE;
    }
}
