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

static void error_callback(int error, const char* description);
static void window_size_callback(GLFWwindow *glfw_window, int x, int y);

static GLFWwindow *window = NULL;

static uint64_t current_time = 0;
static uint64_t current_time_smooth = 0;

static uint64_t dt_hist[9] = { 0 };
static size_t dt_hist_i = 0;
static uint64_t dt_hist_median_smooth = 0;

static uint64_t append_dt_hist(uint64_t dt);
static uint64_t median_dt_hist(void);
static uint64_t qselect(uint64_t *l, size_t n, size_t k);
static size_t qpartition(uint64_t *l, size_t n, size_t p);
static void qswap(uint64_t *a, uint64_t *b);

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

    // Pretend the first frame time was right now
    current_time = kge_timer_now();
    current_time_smooth = current_time;


    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        kprint("GL Error: 0x%x", (int)error);
    }

    return 0;
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

    uint64_t dt = kge_timer_now() - current_time;
    current_time += dt;

    uint64_t dt_median = median_dt_hist();
    uint64_t dt_smooth = 0;
    if (current_time > current_time_smooth)
        dt_smooth = current_time - current_time_smooth;

    if (dt_median != 0) {
        uint64_t nframes = (dt_smooth + dt_median / 2) / dt_median;
        if (nframes < 1)
            nframes = 1;
        dt_smooth = (dt_smooth * 1 + dt_median * nframes * 2) / 3;
        append_dt_hist(dt / nframes);
    }
    else {
        append_dt_hist(dt);
    }
    current_time_smooth += dt_smooth;

    return dt_smooth;
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

static uint64_t append_dt_hist(uint64_t dt)
{
    dt_hist[dt_hist_i++] = dt;
    dt_hist_i %= ARRAY_LEN(dt_hist);
}

static uint64_t median_dt_hist(void)
{
    uint64_t l[ARRAY_LEN(dt_hist)];
    for (size_t i = 0; i < ARRAY_LEN(dt_hist); i++) {
        if (dt_hist[i] == 0)
            return 0;
        l[i] = dt_hist[i];
    }
    uint64_t m = qselect(l, ARRAY_LEN(l), ARRAY_LEN(l) / 2);
    if (dt_hist_median_smooth == 0) {
        dt_hist_median_smooth = m;
    }
    else {
        dt_hist_median_smooth = (m + dt_hist_median_smooth * 31) / 32;
    }
    return dt_hist_median_smooth;
}

static uint64_t qselect(uint64_t *l, size_t n, size_t k)
{
    if (n == 1)
        return l[0];
    size_t p = n / 2;
    p = qpartition(l, n, p);
    if (p == k)
        return l[p];
    else if (p < k)
        return qselect(&l[p + 1], n - (p + 1), k - (p + 1));
    else
        return qselect(l, p, k);
}

static size_t qpartition(uint64_t *l, size_t n, size_t p)
{
    uint64_t tmp;
    uint64_t pivot = l[p];
    size_t s = 0;

    qswap(&l[p], &l[n - 1]);
    for (size_t i = 0; i < n - 1; i++) {
        if (l[i] < pivot) {
            qswap(&l[i], &l[s++]);
        }
    }
    qswap(&l[s], &l[n - 1]);
    return s;
}

static void qswap(uint64_t *a, uint64_t *b)
{
    uint64_t tmp = *a;
    *a = *b;
    *b = tmp;
}

