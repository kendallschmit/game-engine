#include "kge.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "kge_util.h"
#include "kge_timer.h"
#include "vectors.h"
#include "input.h"
#include "shader.h"
#include "texture.h"
#include "vao.h"
#include "draw.h"

#define FRAMERATE_MAX_SMOOTH_RATIO 3

static void error_callback(int error, const char* description);
static void window_size_callback(GLFWwindow *glfw_window, int x, int y);

static GLFWwindow *window = NULL;

static uint64_t prev_frame_time = 0;
static int jitter_count = 0;

static uint64_t delta_time_hist[10] = { 0 };
static uint64_t delta_time_hist_index = 0;

int kge_init(void)
{
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Fatal error: glfwInit() failed\n");
        return -1;
    }

    // Subscribe to GLFW errors
    glfwSetErrorCallback(error_callback);

    // Make a GLFW window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    window = glfwCreateWindow(640, 480, "Your Mom", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }

    // Set up OpenGL context
    glfwMakeContextCurrent(window); // Get context for window
    glfwSwapInterval(1); // Vsync

    // Load OpenGL functions
    gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress);

    // Register callback for window resizes
    glfwSetWindowSizeCallback(window, window_size_callback);

    // Init engine submodules
    input_init(window);
    shader_init();
    vaos_init();
    draw_init(5, 1.5, 0.1, 1000, 6, 1000);

    // Set initial draw size
    int winx, winy;
    glfwGetWindowSize(window, &winx, &winy);
    draw_set_dimensions((GLfloat)winx, (GLfloat)winy);

    // Seed random number generator
    srand(time(NULL));
}

void kge_deinit(void)
{
    // Deinit submodules
    vaos_deinit();
    shader_deinit();

    // Clean up GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}

uint64_t kge_show_frame(void)
{
    glfwSwapBuffers(window);

    // Get time delta, update prev_frame_time
    uint64_t dt = kge_timer_now() - prev_frame_time;
    prev_frame_time += dt;
    // Calculate minimum, average, and maximum past time deltas
    uint64_t dt_min = UINT64_MAX;
    uint64_t dt_max = 0;
    uint64_t dt_avg = 0;
    uint64_t dt_samples = 0;
    for (size_t i = 0; i < ARRAY_LEN(delta_time_hist); i++) {
        uint64_t dti = delta_time_hist[i];
        if (dti == 0)
            continue;
        if (dti < dt_min)
            dt_min = dti;
        if (dti > dt_max)
            dt_max = dti;
        dt_avg += dti;
        dt_samples++;
    }
    if (dt_samples > 0)
        dt_avg /= dt_samples;
    uint64_t dt_min_dev = dt_avg - dt_avg / FRAMERATE_MAX_SMOOTH_RATIO;
    uint64_t dt_max_dev = dt_avg + dt_avg / FRAMERATE_MAX_SMOOTH_RATIO;
    bool hist_good = dt_samples > 0
            && dt_min > dt_min_dev
            && dt_max < dt_max_dev;
    bool dt_good = !hist_good || (dt > dt_min_dev && dt < dt_max_dev);
    //kprint("dt %.3fms, min %.3fms, avg %.3fms, max %.3fms, "
    //        "min_dev %.3fms, max_dev %.3fms, "
    //        "samples %" PRIu64 ", dt %s, hist %s ",
    //        ((float)dt) / 1000000,
    //        ((float)dt_min) / 1000000,
    //        ((float)dt_avg) / 1000000,
    //        ((float)dt_max) / 1000000,
    //        ((float)dt_min_dev) / 1000000,
    //        ((float)dt_max_dev) / 1000000,
    //        dt_samples,
    //        dt_good ? "good" : " bad",
    //        hist_good ? "good" : " bad");
    if (dt_good) {
        delta_time_hist[delta_time_hist_index] = dt;
        dt = dt_avg;
    }
    else {
        delta_time_hist[delta_time_hist_index] = 0;
    }
    delta_time_hist_index++;
    delta_time_hist_index %= ARRAY_LEN(delta_time_hist);


    glClear(GL_COLOR_BUFFER_BIT
            | GL_DEPTH_BUFFER_BIT
            | GL_STENCIL_BUFFER_BIT);

    glfwPollEvents();

    return dt;
}

uint64_t kge_prev_frame_time(void)
{
    return prev_frame_time;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Fatal error: %s\n", description);
    exit(EXIT_FAILURE);
}

static void window_size_callback(GLFWwindow *glfw_window, int x, int y)
{
    draw_set_dimensions((GLfloat)x, (GLfloat)y);
}
