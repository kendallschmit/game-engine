#include "vao.h"

#include "stddef.h"

GLuint vaos[VAO_MAX] = { 0 };
GLuint vao_inst_offsets_buf = 0;

void vaos_init() {
    // Buffer for instance offsets
    glGenBuffers(1, &vao_inst_offsets_buf);
    glBindBuffer(GL_ARRAY_BUFFER, vao_inst_offsets_buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * VAO_INST_MAX * 3,
            NULL, GL_DYNAMIC_DRAW);

    // VAO for quad
    GLuint vao; // Name of VAO
    glGenVertexArrays(1, &vao); // Generate one VAO
    glBindVertexArray(vao); // Make it active VAO

    // lower left quad
    //GLfloat quad_buf[] = {
    //    0.0, 0.0, 0.0,   0.0, 1.0,   // BL
    //    1.0, 0.0, 0.0,   1.0, 1.0,   // BR
    //    0.0, 1.0, 0.0,   0.0, 0.0,   // TL

    //    1.0, 1.0, 0.0,   1.0, 0.0,   // TR
    //    0.0, 1.0, 0.0,   0.0, 0.0,   // TL
    //    1.0, 0.0, 0.0,   1.0, 1.0,   // BR
    //}; */
    // centered quad
    //GLfloat quad_buf[] = {
    //    -0.5, -0.5, 0.0,   0.0, 1.0,   // BL
    //     0.5, -0.5, 0.0,   1.0, 1.0,   // BR
    //    -0.5,  0.5, 0.0,   0.0, 0.0,   // TL
    //     0.5,  0.5, 0.0,   1.0, 0.0,   // TR
    //    -0.5,  0.5, 0.0,   0.0, 0.0,   // TL
    //     0.5, -0.5, 0.0,   1.0, 1.0,   // BR
    //};
    // centered quad, triangle strip
    GLfloat quad_buf[] = {
        -0.5,  0.5, 0.0,   0.0, 0.0,   // TL
        -0.5, -0.5, 0.0,   0.0, 1.0,   // BL
         0.5,  0.5, 0.0,   1.0, 0.0,   // TR
         0.5, -0.5, 0.0,   1.0, 1.0,   // BR
    };
    // VERTEX BUFFER
    GLuint buf; // Name of vertex buffer
    glGenBuffers(1, &buf); // Generate buffer

    // glVertexAttribPointer(index, size, type, normalized, stride, offset)

    // buf should contain position and uv coords
    glBindBuffer(GL_ARRAY_BUFFER, buf); // Make it the active ARRAY_BUFFER
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
    glBindBuffer(GL_ARRAY_BUFFER, vao_inst_offsets_buf);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 3, 0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // DONE MAKING CHANGES
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // Store it
    vaos[VAO_QUAD] = vao;

}

void vaos_deinit() {
    glDeleteVertexArrays(VAO_MAX, vaos);
}
