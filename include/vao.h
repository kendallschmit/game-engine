#ifndef VAO_H
#define VAO_H

#include <glad/glad.h>

#define VAO_INST_MAX 100000
#define VAO_LINE_VALUES_MAX 1000

enum vaos {
    VAO_QUAD = 0,
    VAO_LINE,
    VAO_MAX,
};

enum vao_buffers {
    VAO_BUFFERS_QUAD = 0,
    VAO_BUFFERS_LINE = 1,
    VAO_BUFFERS_INST_OFFSETS,
    VAO_BUFFERS_LINE_VALUES,
    VAO_BUFFERS_MAX,
};

extern GLuint vaos[VAO_MAX];
extern GLuint vao_buffers[VAO_BUFFERS_MAX];

void vaos_init(void);
void vaos_deinit(void);

#endif
