#include "kdraw.h"

#include <string.h>
#include <math.h>

#include "kutil.h"
#include "shad.h"

#define OBJ_MAX 1000
// kdraw objects
static size_t nobjects[KDRAW_PROJ_MAX];
static struct kdraw objects[KDRAW_PROJ_MAX][OBJ_MAX];

// Projection matrices for each projection type
// Maybe switch to different pipelines/shaders instead of just projection?
static GLfloat projmats[KDRAW_PROJ_MAX][16]; // Holds the projection matrices
static GLfloat viewmat[16];

// references to shader args
GLint modelmatuloc;
GLint viewmatuloc;
GLint projmatuloc;

// Static
static GLfloat identity4[] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0,
};

static void print_mat(GLfloat *mat) {
    for (int i = 0; i < 4; i++) {
        int k = 4 * i;
        kprint("%g %g %g %g\n",
                mat[k], mat[k + 1], mat[k + 2], mat[k + 3]);
    }
}

// Each row is an x, y, z, w
#define mat_set(mat, r, c, v) do { (mat)[(r) * 4 + (c)] = (v); } while(0);
static void gen_projmat(GLfloat *mat, GLfloat angle, GLfloat ratio,
        GLfloat near, GLfloat far)
{
    GLfloat tan_half_angle;
    tan_half_angle = tan(angle / 2);
    mat_set(mat, 0, 0, 1 / (ratio * tan_half_angle));
    mat_set(mat, 1, 1, 1 / (tan_half_angle));
    mat_set(mat, 2, 2, -(far + near) / (far - near));
    mat_set(mat, 2, 3, -1);
    mat_set(mat, 3, 2, -(2 * far * near) / (far - near));
}

// Extern
extern void kdraw_init(GLFWwindow *window)
{
    glUseProgram(shad_simple_prog);
    modelmatuloc = glGetUniformLocation(shad_simple_prog, "modelmat");
    viewmatuloc = glGetUniformLocation(shad_simple_prog, "viewmat");
    projmatuloc = glGetUniformLocation(shad_simple_prog, "projmat");
    glEnable(GL_DEPTH_TEST);

    // View matrix
    memcpy(viewmat, identity4, sizeof(viewmat));
    mat_set(viewmat, 3, 2, -VIEW_DIST);
    glUniformMatrix4fv(viewmatuloc, 1, GL_FALSE, viewmat);

    // Need to adapt to the aspect ration when we start
    kdraw_window_adapt(window);
}

extern void kdraw_window_adapt(GLFWwindow *window) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    GLfloat ratio = (GLfloat)w / h;

    // Projection matrix (perspective)
    memcpy(projmats[KDRAW_PROJ_PERSP], identity4, sizeof(identity4));
    gen_projmat(projmats[KDRAW_PROJ_PERSP], FOV_RAD, ratio,
            NEAR_CLIP, FAR_CLIP);

    // Projection matrix (orthographic)
    memcpy(projmats[KDRAW_PROJ_ORTHO], identity4, sizeof(identity4));
    if (w > h) {
        mat_set(projmats[KDRAW_PROJ_ORTHO], 0, 0, 1 / (GLfloat)w / h / ORTHO_MIN);
        mat_set(projmats[KDRAW_PROJ_ORTHO], 1, 1, 1 / ORTHO_MIN);
        mat_set(projmats[KDRAW_PROJ_ORTHO], 2, 2, 1 / ratio / 6.0);
    }
    else {
        mat_set(projmats[KDRAW_PROJ_ORTHO], 0, 0, 1 / (GLfloat)w / h / ORTHO_MIN);
        mat_set(projmats[KDRAW_PROJ_ORTHO], 1, 1, 1 / ORTHO_MIN);
        mat_set(projmats[KDRAW_PROJ_ORTHO], 2, 2, 1 / ratio / 6.0);
    }
}

extern void kdraw_draw(GLFWwindow *window)
{
    glClear(GL_COLOR_BUFFER_BIT
            | GL_DEPTH_BUFFER_BIT
            | GL_ACCUM_BUFFER_BIT
            | GL_STENCIL_BUFFER_BIT);

    // Model matrix (changed for each model)
    GLfloat modelmat[16];
    memcpy(modelmat, identity4, sizeof(identity4));
    for (int p = KDRAW_PROJ_START; p < KDRAW_PROJ_MAX; p++) {
        glUniformMatrix4fv(projmatuloc, 1, GL_FALSE, projmats[p]);
        for (GLuint i = 0; i < nobjects[p]; i++) {
            // Assign model matrix
            mat_set(modelmat, 3, 0, objects[p][i].pos.x);
            mat_set(modelmat, 3, 1, objects[p][i].pos.y);
            mat_set(modelmat, 3, 2, objects[p][i].pos.z);
            glUniformMatrix4fv(modelmatuloc, 1, GL_FALSE, modelmat);

            // Draw model
            glBindVertexArray(objects[p][i].vao);
            glBindTexture(GL_TEXTURE_2D, objects[p][i].tex);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glfwSwapBuffers(window);
}

extern struct kdraw *kdraw_make_quad(size_t tex, int proj)
{
    // VAO
    GLuint vao; // Name of VAO
    glGenVertexArrays(1, &vao); // Generate one VAO
    glBindVertexArray(vao); // Make it active VAO

    GLfloat quad_buf[] = {
        0.0, 0.0, 0.0,   0.0, 1.0,   // BL
        1.0, 0.0, 0.0,   1.0, 1.0,   // BR
        0.0, 1.0, 0.0,   0.0, 0.0,   // TL

        1.0, 1.0, 0.0,   1.0, 0.0,   // TR
        0.0, 1.0, 0.0,   0.0, 0.0,   // TL
        1.0, 0.0, 0.0,   1.0, 1.0,   // BR
    };

    // VERTEX BUFFER
    GLuint buf; // Name of vertex buffer
    glGenBuffers(1, &buf); // Generate buffer
    glBindBuffer(GL_ARRAY_BUFFER, buf); // Make it the active ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_buf), quad_buf, GL_STATIC_DRAW);
    // Set x, y positions for tris
    // index, size, type, normalized, stride, offset
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, 0);
    glEnableVertexAttribArray(0); // Enable VA at index 0
    // Set uv positions for tris
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, (GLvoid *)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1); // Enable VA at index 1

    // DONE MAKING CHANGES
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // TRACK VAO
    objects[proj][nobjects[proj]] = (struct kdraw) {
        .vao = vao,
        .tex = tex,
    };
    return &objects[proj][nobjects[proj]++];
}
