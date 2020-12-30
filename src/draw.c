#include "draw.h"

#include <string.h>
#include <math.h>

#include "shader.h"
#include "kge_util.h"

#define OBJ_MAX 100000

// Identity matrix used to reset other matrices
static GLfloat identity4[] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0,
};

// Used for debugging matrices
static void print_mat(GLfloat *mat)
{
    for (int i = 0; i < 4; i++) {
        int k = 4 * i;
        kprint("%g %g %g %g\n",
                mat[k], mat[k + 1], mat[k + 2], mat[k + 3]);
    }
}

// Each row is an x, y, z, w
#define mat_set(mat, r, c, v) do { (mat)[(r) * 4 + (c)] = (v); } while(0);

// Generate a perspective projection matrix
static void gen_perspective_matrix(GLfloat *mat, GLfloat angle, GLfloat ratio,
        GLfloat near, GLfloat far)
{
    memcpy(mat, identity4, sizeof(identity4));
    GLfloat tan_half_angle;
    tan_half_angle = tan(angle / 2);
    mat_set(mat, 0, 0, 1 / (ratio * tan_half_angle));
    mat_set(mat, 1, 1, 1 / (tan_half_angle));
    mat_set(mat, 2, 2, -(far + near) / (far - near));
    mat_set(mat, 2, 3, -1);
    mat_set(mat, 3, 2, -(2 * far * near) / (far - near));
}

// Generate an orthographic projection matrix
static void gen_orthographic_matrix(GLfloat *mat, GLfloat ortho_width,
        GLfloat ortho_depth, GLfloat w, GLfloat h)
{
    memcpy(mat, identity4, sizeof(identity4));
    if (w > h) {
        mat_set(mat, 0, 0, (1.0 / ortho_width) * h / w);
        mat_set(mat, 1, 1, 1.0 / ortho_width);
        mat_set(mat, 2, 2, 1.0 / ortho_depth);
    }
    else {
        mat_set(mat, 0, 0, 1.0 / ortho_width);
        mat_set(mat, 1, 1, (1.0 / ortho_width) * w / h);
        mat_set(mat, 2, 2, 1.0 / ortho_depth);
    }
}

// Projection matrices
static GLfloat orthographic_matrix[16];
static GLfloat perspective_matrix[16];
static GLfloat view_matrix[16];

// References to shader args for projection matrices
static GLint model_matrix_location;
static GLint view_matrix_location;
static GLint proj_matrix_location;

static GLfloat view_distance;
static GLfloat fov_rad;
static GLfloat near_clip;
static GLfloat far_clip;
static GLfloat ortho_width;
static GLfloat ortho_depth;

void draw_init(GLfloat view_distance_a, GLfloat fov_rad_a,
        GLfloat near_clip_a, GLfloat far_clip_a, GLfloat ortho_width_a,
        GLfloat ortho_depth_a)
{
    view_distance = view_distance_a;
    fov_rad = fov_rad_a;
    near_clip = near_clip_a;
    far_clip = far_clip_a;
    ortho_width = ortho_width_a;
    ortho_depth = ortho_depth_a;

    glEnable(GL_DEPTH_TEST);

    // Use simple shader and figure out where the parameters go
    glUseProgram(shader_program_simple);
    model_matrix_location = glGetUniformLocation(shader_program_simple,
            "modelmat");
    view_matrix_location = glGetUniformLocation(shader_program_simple,
            "viewmat");
    proj_matrix_location = glGetUniformLocation(shader_program_simple,
            "projmat");

    // Can generate view matrix now because camera never moves
    memcpy(view_matrix, identity4, sizeof(view_matrix));
    mat_set(view_matrix, 3, 2, -view_distance);
    glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, view_matrix);
}

void draw_set_dimensions(GLuint w, GLuint h)
{
    // Have to regenerat projection matrices for new aspect ratio
    GLfloat ratio = (GLfloat)w / h;
    gen_perspective_matrix(perspective_matrix, fov_rad, ratio,
            near_clip, far_clip);
    gen_orthographic_matrix(orthographic_matrix, ortho_width,
            ortho_depth, w, h);
}

void draw_list(struct draw *draws, GLuint ndraws, GLuint projection,
        bool same_vao, bool same_tex)
{
    // Set projection matrix
    GLfloat *proj_matrix = identity4;
    switch (projection) {
        case PROJECTION_ORTHOGRAPHIC:
            proj_matrix = orthographic_matrix;
            break;
        case PROJECTION_PERSPECTIVE:
            proj_matrix = perspective_matrix;
            break;
        default:
            kprint("Invalid projection type");
            return;
    }
    glUniformMatrix4fv(proj_matrix_location, 1, GL_FALSE, proj_matrix);

    GLfloat model_matrix[16];
    memcpy(model_matrix, identity4, sizeof(identity4));
    for (GLuint i = 0; i < ndraws; i++) {
        struct draw *d = &draws[i];
        // Set up model matrix
        mat_set(model_matrix, 3, 0, d->pos.x);
        mat_set(model_matrix, 3, 1, d->pos.y);
        mat_set(model_matrix, 3, 2, d->pos.z);
        glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, model_matrix);
        // Draw
        if (!same_vao || i == 0)
            glBindVertexArray(d->vao);
        if (!same_tex || i == 0)
            glBindTexture(GL_TEXTURE_2D, d->tex);
        glDrawArrays(GL_TRIANGLES, 0, 6); // TODO Not always 6
    }
    // Unbind everything to clean up
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (projection == PROJECTION_ORTHOGRAPHIC)
        return;
}
