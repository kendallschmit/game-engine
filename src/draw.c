#include "draw.h"

#include <string.h>
#include <math.h>

#include "shader.h"
#include "vao.h"
#include "kge_util.h"

static GLfloat inst_offsets[3 * VAO_INST_MAX] = { 0 };

// Identity matrix used to reset other matrices
static GLfloat identity4[] = {
    1.0, 0.0, 0.0, 0.0, // This is *probably* a column going left to right...
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
#define mat_set(mat, c, r, v) do { (mat)[(c) * 4 + (r)] = (v); } while(0);

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
static GLint view_matrix_location;
static GLint proj_matrix_location;

static GLint line_position_location;
static GLint line_scale_location;

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
    view_matrix_location = glGetUniformLocation(shader_program_simple,
            "viewmat");
    proj_matrix_location = glGetUniformLocation(shader_program_simple,
            "projmat");

    // Line shader
    line_position_location = glGetUniformLocation(shader_program_line,
            "position");
    line_scale_location = glGetUniformLocation(shader_program_line,
            "scale");

    // Can generate view matrix now because camera never moves
    glUseProgram(shader_program_simple);
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

void draw_clear(void)
{
    glClear(GL_COLOR_BUFFER_BIT
            | GL_DEPTH_BUFFER_BIT
            | GL_STENCIL_BUFFER_BIT);
}

void draw_list(GLuint vao, GLuint tex, struct draw *draws, GLuint ndraws,
        GLuint projection)
{
    glUseProgram(shader_program_simple);

    while (ndraws > VAO_INST_MAX) {
        draw_list(vao, tex, draws, VAO_INST_MAX, projection);
        draws = &draws[VAO_INST_MAX];
        ndraws -= VAO_INST_MAX;
    }
    if (ndraws < 1)
        return;

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

    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, tex);

    for (GLuint i = 0; i < ndraws; i++) {
        struct draw *d = &draws[i];
        inst_offsets[i * 3 + 0] = d->pos.x;
        inst_offsets[i * 3 + 1] = d->pos.y;
        inst_offsets[i * 3 + 2] = d->pos.z;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_INST_OFFSETS]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * ndraws,
            inst_offsets);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ndraws);

    // Unbind everything to clean up
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_lines(struct vec2 position, struct vec2 scale,
        GLfloat *values, GLuint nvalues)
{
    if (nvalues < 2)
        return;
    glUseProgram(shader_program_line);

    glUniform2f(line_position_location, position.x, position.y);
    glUniform2f(line_scale_location, scale.x, scale.y);

    glBindVertexArray(vaos[VAO_LINE]);

    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_LINE_VALUES]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * nvalues, values);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArraysInstanced(GL_LINES, 0, 2, nvalues - 1);

    // Unbind everything to clean up
    glBindVertexArray(0);
}
