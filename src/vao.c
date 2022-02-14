#include "vao.h"

#include <stddef.h>

#include "kge_util.h"

GLuint vaos[VAO_MAX] = { 0 };
GLuint vao_buffers[VAO_BUFFERS_MAX] = { 0 };

static void init_quad(void);
static void init_line(void);

void vaos_init(void) {
    glGenVertexArrays(VAO_MAX, vaos);
    glGenBuffers(VAO_BUFFERS_MAX, vao_buffers);
    init_quad();
    init_line();
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        kprint("GL Error: 0x%x", (int)error);
    }
}

void vaos_deinit(void) {
    glDeleteVertexArrays(VAO_MAX, vaos);
    glDeleteBuffers(VAO_BUFFERS_MAX, vao_buffers);
}

static void init_quad(void)
{
    // VAO for quad
    glBindVertexArray(vaos[VAO_QUAD]); // Make it active VAO

    // centered quad, triangle strip
    GLfloat quad_buf[] = {
        -0.5,  0.5, 0.0,   0.0, 0.0,   // TL
        -0.5, -0.5, 0.0,   0.0, 1.0,   // BL
         0.5,  0.5, 0.0,   1.0, 0.0,   // TR
         0.5, -0.5, 0.0,   1.0, 1.0,   // BR
    };
    // VERTEX BUFFER

    // glVertexAttribPointer(index, size, type, normalized, stride, offset)

    // Buffer should contain position and uv coords
    // Make it the active ARRAY_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_QUAD]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_buf), quad_buf, GL_STATIC_DRAW);
    // Set x, y positions for tris
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, 0);
    glEnableVertexAttribArray(0); // Enable VA at index 0
    // Set uv positions for tris
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 5, (GLvoid *)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1); // Enable VA at index 1

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // buf_offset should contain xyz offset for instances
    glBindBuffer(GL_ARRAY_BUFFER, vao_buffers[VAO_BUFFERS_INST_OFFSETS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * VAO_INST_MAX * 3,
            NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // DONE MAKING CHANGES
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void init_line(void)
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * VAO_LINE_VALUES_MAX, NULL,
            GL_DYNAMIC_DRAW);
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
