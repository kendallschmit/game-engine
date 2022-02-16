#include "draw.h"

#include <string.h>
#include <math.h>

#include "shader.h"
#include "kge_util.h"

static GLuint screen_width = 1;
static GLuint screen_height = 1;

// Identity matrix used to reset other matrices
static GLfloat identity4[] = {
    1.0, 0.0, 0.0, 0.0, // This line is first column from top to bottom
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0,
};

// Used for debugging matrices
static void print_mat(GLfloat *mat)
{
    kprint("::matrix::");
    for (int i = 0; i < 4; i++) {
        kprint("% 6.02f % 6.02f % 6.02f % 6.02f",
                mat[i + 0], mat[i + 4], mat[i + 8], mat[i + 12]);
    }
}

// Each row is an x, y, z, w
#define mat_set(mat, r, c, v) do { (mat)[(c) * 4 + (r)] = (v); } while(0);

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
    mat_set(mat, 3, 2, -1);
    mat_set(mat, 2, 3, (2 * far * near) / (far - near));
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
static GLint line_screen_dimensions_location;
static GLint line_radius_location;
static GLint line_in_color_location;

static GLfloat view_distance;
static GLfloat fov_rad;
static GLfloat near_clip;
static GLfloat far_clip;
static GLfloat ortho_width;
static GLfloat ortho_depth;

// Models
GLuint draw_model_quad;
GLuint draw_model_cube;

struct model {
    bool exists;
    GLuint vao_name;
    GLuint buf_name;
    GLuint nverts;
    GLenum mode;
};
static struct model models[32] = { 0 };

// VAOs
#define VAO_INST_MAX 100000
#define VAO_LINE_VALUES_MAX 1000

static GLfloat inst_offsets[3 * VAO_INST_MAX] = { 0 };

enum vaos {
    VAO_LINE,
    VAO_MAX,
};
static GLuint vaos[VAO_MAX] = { 0 };
enum vao_buffers {
    VAO_BUFFERS_INST_OFFSETS,
    VAO_BUFFERS_LINE,
    VAO_BUFFERS_LINE_VALUES,
    VAO_BUFFERS_MAX,
};
static GLuint vao_buffers[VAO_BUFFERS_MAX] = { 0 };

static void init_vaos(void);
static void init_line_vao(void);

void draw_init(GLfloat view_distance_a, GLfloat fov_rad_a,
        GLfloat near_clip_a, GLfloat far_clip_a, GLfloat ortho_width_a,
        GLfloat ortho_depth_a)
{
    init_vaos();

    view_distance = view_distance_a;
    fov_rad = fov_rad_a;
    near_clip = near_clip_a;
    far_clip = far_clip_a;
    ortho_width = ortho_width_a;
    ortho_depth = ortho_depth_a;

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
    line_screen_dimensions_location = glGetUniformLocation(shader_program_line,
            "screen_dimensions");
    line_radius_location = glGetUniformLocation(shader_program_line,
            "radius");
    line_in_color_location = glGetUniformLocation(shader_program_line,
            "in_color");

    // TODO Init view position

    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        kprint("GL Error: 0x%x", (int)error);
    }
}

void draw_set_dimensions(GLuint w, GLuint h)
{
    screen_width = w;
    screen_height = h;
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

void draw_look_at(struct vec3 origin, struct vec3 target, struct vec3 up)
{
    glUseProgram(shader_program_simple);
    memcpy(view_matrix, identity4, sizeof(view_matrix));

    //mat_set(view_matrix, 2, 3, -view_distance);

    mat_set(view_matrix, 0, 3, -origin.x);
    mat_set(view_matrix, 1, 3, -origin.y);
    mat_set(view_matrix, 2, 3, -origin.z);

    struct vec3 z_axis = vec3_norm(vec3_diff(origin, target));
    struct vec3 x_axis = vec3_norm(vec3_cross(up, z_axis));
    struct vec3 y_axis = vec3_cross(z_axis, x_axis);

    mat_set(view_matrix, 0, 0, x_axis.x);
    mat_set(view_matrix, 1, 0, y_axis.x);
    mat_set(view_matrix, 2, 0, z_axis.x);

    mat_set(view_matrix, 0, 1, x_axis.y);
    mat_set(view_matrix, 1, 1, y_axis.y);
    mat_set(view_matrix, 2, 1, z_axis.y);

    mat_set(view_matrix, 0, 2, x_axis.z);
    mat_set(view_matrix, 1, 2, y_axis.z);
    mat_set(view_matrix, 2, 2, z_axis.z);

    mat_set(view_matrix, 0, 3, -vec3_dot(x_axis, origin));
    mat_set(view_matrix, 1, 3, -vec3_dot(y_axis, origin));
    mat_set(view_matrix, 2, 3, -vec3_dot(z_axis, origin));

    glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, view_matrix);

    //print_mat(view_matrix);
}

GLuint draw_add_model(GLfloat *verts, GLint size, GLenum mode)
{
    struct model *model = NULL;
    GLuint model_index;
    for (model_index = 0; model_index < ARRAY_LEN(models); model_index++) {
        if (!models[model_index].exists) {
            model = &models[model_index];
            break;
        }
    }
    if (model == NULL) {
        // TODO Something better for this error?
        kprint("Error: Models list at capacity");
        return model_index;
    }

    model->exists = true;
    model->nverts = size / (sizeof(GLfloat) * 5);
    model->mode = mode;

    glGenVertexArrays(1, &model->vao_name); // Generate vertex array object
    glBindVertexArray(model->vao_name); // Make it active

    glGenBuffers(1, &model->buf_name); // Generate buffer for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, model->buf_name); // Make it active

    // Copy data to buffer
    glBufferData(GL_ARRAY_BUFFER, size, verts, GL_STATIC_DRAW);
    // Define layout of vertex data: [x, y, z, u, v]
    // Index 0 (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, 0);
    glEnableVertexAttribArray(0);
    // Index 1 (u, v)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, (GLvoid *)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1);

    // Index 2 instance offsets [x, y, z]
    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_INST_OFFSETS]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // Done setting things up for this model, so unbind everything
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return model_index;
}

static void draw_models_raw(GLuint model, struct draw *draws, GLuint ndraws)
{
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

    glDrawArraysInstanced(models[model].mode, 0, models[model].nverts, ndraws);
}

void draw_models(GLuint model, GLuint tex,
        struct draw *draws, GLuint ndraws,
        GLuint projection)
{
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shader_program_simple);

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

    glBindVertexArray(models[model].vao_name);
    glBindTexture(GL_TEXTURE_2D, tex);

    while (ndraws > VAO_INST_MAX) {
        draw_models_raw(model, draws, VAO_INST_MAX);
        draws = &draws[VAO_INST_MAX];
        ndraws -= VAO_INST_MAX;
    }
    if (ndraws > 0)
    {
        draw_models_raw(model, draws, ndraws);
    }

    // Unbind everything to clean up
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_lines(struct vec2 position, struct vec2 scale, GLfloat width,
        struct vec3 color, GLfloat *values, GLuint nvalues)
{
    if (nvalues < 2)
        return;
    glDisable(GL_DEPTH_TEST);

    glUseProgram(shader_program_line);

    glUniform2f(line_position_location, position.x, position.y);
    glUniform2f(line_scale_location, scale.x, scale.y);
    glUniform2f(line_screen_dimensions_location, screen_width, screen_height);
    glUniform1f(line_radius_location, width / 2);
    glUniform4f(line_in_color_location, color.x, color.y, color.z, 1.0);

    glBindVertexArray(vaos[VAO_LINE]);

    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_LINE_VALUES]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * nvalues, values);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, nvalues - 1);

    // Unbind everything to clean up
    glBindVertexArray(0);
}

static void init_vaos(void)
{
    glGenVertexArrays(VAO_MAX, vaos);
    glGenBuffers(VAO_BUFFERS_MAX, vao_buffers);

    // Buffer for instance offsets [x, y, z]
    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_INST_OFFSETS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * VAO_INST_MAX * 3,
            NULL, GL_DYNAMIC_DRAW);

    // Buffer for line values [v]
    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_LINE_VALUES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * VAO_LINE_VALUES_MAX, NULL,
            GL_DYNAMIC_DRAW);

    // Done setting up buffers, unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Model: quad
    GLfloat quad_verts[] = {
        -0.5,  0.5, 0.0,   0.0, 0.0,   // TL
        -0.5, -0.5, 0.0,   0.0, 1.0,   // BL
         0.5,  0.5, 0.0,   1.0, 0.0,   // TR
         0.5, -0.5, 0.0,   1.0, 1.0,   // BR
    };
    draw_model_quad = draw_add_model(quad_verts, sizeof(quad_verts),
            GL_TRIANGLE_STRIP);

    // Model: cube
    //
    //     (A)-------------(C)
    //     /|              /|
    //    / |             / |
    //   /  |            /  |
    // (B)-------------(D)  |
    //  |   |           |   |
    //  |  (E)--------- | -(G)
    //  |  /            |  /
    //  | /             | /
    //  |/              |/
    // (F)-------------(H)
    //
    GLfloat cube_verts[] = {
        -0.5,  0.5, -0.5,   0.0,  0.0,   // A
        -0.5,  0.5,  0.5,   0.0,  0.0,   // B
         0.5,  0.5, -0.5,   1.0,  0.0,   // C
         0.5,  0.5,  0.5,   1.0,  0.0,   // D
         0.5, -0.5,  0.5,   1.0,  1.0,   // H
        -0.5,  0.5,  0.5,   0.0,  0.0,   // B
        -0.5, -0.5,  0.5,   0.0,  1.0,   // F
        -0.5, -0.5, -0.5,   0.0,  1.0,   // E
         0.5, -0.5,  0.5,   1.0,  1.0,   // H
         0.5, -0.5, -0.5,   1.0,  1.0,   // G
         0.5,  0.5, -0.5,   1.0,  0.0,   // C
        -0.5, -0.5, -0.5,   0.0,  1.0,   // E
        -0.5,  0.5, -0.5,   0.0,  0.0,   // A
        -0.5,  0.5,  0.5,   0.0,  0.0,   // B
    };
    draw_model_cube = draw_add_model(cube_verts, sizeof(cube_verts),
            GL_TRIANGLE_STRIP);

    init_line_vao();
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        kprint("GL Error: 0x%x", (int)error);
    }

    // TODO Deinit function
    //void vaos_deinit(void) {
    //    glDeleteVertexArrays(VAO_MAX, vaos);
    //    glDeleteBuffers(VAO_BUFFERS_MAX, vao_buffers);
    //}
}

static void init_line_vao(void)
{
    glBindVertexArray(vaos[VAO_LINE]); // Make it active VAO

    // Line through center
    GLfloat line_buf[] = {
        0.0, 0.0, 1.0, 1.0,
    };
    // VERTEX BUFFER
    // glVertexAttribPointer(index, size, type, normalized, stride, offset)
    // buf should contain position and uv coords
    // Make it the active ARRAY_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_LINE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_buf), line_buf, GL_STATIC_DRAW);
    // Set x positions for points
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0); // Enable VA at index 0
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // This buffer contains y values for points
    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_LINE_VALUES]);
    // First attribute starts at index 0
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    // Second attribute starts at index 1
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat),
            (GLvoid *)sizeof(GLfloat));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // DONE MAKING CHANGES
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
