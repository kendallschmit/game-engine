#include "engine.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

// Engine stuff
struct input_set {
    bool up;
    bool down;
    bool left;
    bool right;
};
struct input_set input = { 0 };

#define OBJECTS_MAX 10000
static GLuint northo_objs = 0;
struct draw ortho_draws[OBJECTS_MAX] = { 0 };
static struct kge_obj ortho_objs[OBJECTS_MAX] = { 0 };

static GLuint npersp_objs = 0;
struct draw persp_draws[OBJECTS_MAX] = { 0 };
static struct kge_obj persp_objs[OBJECTS_MAX] = { 0 };


static GLuint tex_akko;
static GLuint tex_ritsu;

// Input/main thread stuff
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    bool *val;
    switch (key) {
        case GLFW_KEY_UP:
            val = &input.up;
            break;
        case GLFW_KEY_DOWN:
            val = &input.down;
            break;
        case GLFW_KEY_LEFT:
            val = &input.left;
            break;
        case GLFW_KEY_RIGHT:
            val = &input.right;
            break;
        default:
            return;
    }
    if (action == GLFW_PRESS) {
        *val = true;
    }
    else if (action == GLFW_RELEASE) {
        *val = false;
    }
}

static void main_thread(GLFWwindow *window, struct kge_thread *render_thread)
{
    // Set up game
    srand(time(NULL));

    kge_thread_lock(render_thread); // Lock render thread during init
    // Only orthographic object is player for now
    kprint("Set up player");
    struct kge_obj *player = &ortho_objs[0];
    player->draw = &ortho_draws[0];
    *player->draw = (struct draw){
        .vao = vaos[VAO_QUAD],
        .tex = tex_akko,
    };
    northo_objs = 1;

    // Ritsus are all perspective
    kprint("Set up ritsus");
    GLuint nritsus = 5000;
    for (GLuint i = 0; i < nritsus; i++) {
        persp_objs[i].draw = &persp_draws[i];
        *persp_objs[i].draw = (struct draw){
            .vao = vaos[VAO_QUAD],
            .tex = tex_ritsu,
        };
        persp_objs[i].pos = (struct vec3i) {
            randi(-1000 * SCALE, 1000 * SCALE),
            randi(-1000 * SCALE, 1000 * SCALE),
            randi(-1000 * SCALE, 10 * SCALE),
        };
        persp_objs[i].vel.z = 80;
        //persp_objs[i].vel.y = 80;
    }
    npersp_objs = nritsus;
    kge_thread_unlock(render_thread); // Render thread is safe to go

    // Loop
    struct kge_timer input_timer;
    kge_timer_start(&input_timer);
    while (!glfwWindowShouldClose(window)) {
        kge_thread_lock(render_thread);
        // Input
        glfwPollEvents();
        player->vel = (struct vec3i){ 0, 0, 0 };
        if (input.up)
            player->vel.y = 3;
        if (input.down)
            player->vel.y = -3;
        if (input.left)
            player->vel.x = -3;
        if (input.right)
            player->vel.x = 3;

        // Physics (all integers)
        uint64_t phys_time = kge_timer_reset(&input_timer) / (NANOS / SCALE);
        for (int i = 0; i < northo_objs; i++) {
            ortho_objs[i].pos.x += ortho_objs[i].vel.x * phys_time;
            ortho_objs[i].pos.y += ortho_objs[i].vel.y * phys_time;
            ortho_objs[i].pos.z += ortho_objs[i].vel.z * phys_time;
        }
        kge_thread_unlock(render_thread);

        //kprint("Time between inputs %08.04fms", (double)phys_time / 1000);
        struct timespec ts = { 0, 4 * (NANOS / MILLIS) };
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
    struct kge_timer render_timer;
    kge_timer_start(&render_timer);
    while (!thread->terminated) {
        // Physics for perspective objects
        uint64_t phys_time = kge_timer_reset(&render_timer) / (NANOS / SCALE);
        for (int i = 0; i < npersp_objs; i++) {
            persp_objs[i].pos.x += persp_objs[i].vel.x * phys_time;
            persp_objs[i].pos.y += persp_objs[i].vel.y * phys_time;
            persp_objs[i].pos.z += persp_objs[i].vel.z * phys_time;
            if (persp_objs[i].pos.z > 10 * SCALE)
                persp_objs[i].pos.z -= 1010 * SCALE;
            if (persp_objs[i].pos.y > 1000 * SCALE)
                persp_objs[i].pos.y -= 2000 * SCALE;
        }

        // Calculate kdraw values
        kge_thread_lock(thread);
        for (GLuint i = 0; i < npersp_objs; i++) {
            struct kge_obj *e = &persp_objs[i];
            e->draw->pos.x = (GLfloat)e->pos.x / SCALE;
            e->draw->pos.y = (GLfloat)e->pos.y / SCALE;
            e->draw->pos.z = (GLfloat)e->pos.z / SCALE;
        }
        for (GLuint i = 0; i < northo_objs; i++) {
            struct kge_obj *e = &ortho_objs[i];
            e->draw->pos.x = (GLfloat)e->pos.x / SCALE;
            e->draw->pos.y = (GLfloat)e->pos.y / SCALE;
            e->draw->pos.z = (GLfloat)e->pos.z / SCALE;
        }
        kge_thread_unlock(thread);

        // Draw and swap
        glClear(GL_COLOR_BUFFER_BIT
                | GL_DEPTH_BUFFER_BIT
                | GL_ACCUM_BUFFER_BIT
                | GL_STENCIL_BUFFER_BIT);
        draw_list(persp_draws, npersp_objs, PROJECTION_PERSPECTIVE,
                false, false);
        draw_list(ortho_draws, northo_objs, PROJECTION_ORTHOGRAPHIC,
                false, false);
        glfwSwapBuffers(args->window);
        uint64_t render_time = kge_timer_interval(&render_timer);
        //kprint("Render time %08.04fms",
        //        (double)render_time / (NANOS / MILLIS));
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
    glfwSetKeyCallback(window, key_callback);

    // Start render thread
    struct render_thread_args render_thread_args = { .window = window };
    struct kge_thread render_thread = { .arg = &render_thread_args };
    if (kge_thread_start(&render_thread, "/renderer", render_thread_fn) != 0) {
        kprint("Unable to start render thread");
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
