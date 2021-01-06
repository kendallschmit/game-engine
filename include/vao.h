#ifndef VAO_H
#define VAO_H

#include <glad/glad.h>

#define VAO_QUAD 0
#define VAO_MAX 1

#define VAO_INST_MAX 100000

extern GLuint vaos[VAO_MAX];
extern GLuint vao_inst_offsets_buf;

void vaos_init();
void vaos_deinit();

#endif
