#ifndef VAO_H
#define VAO_H

#include <glad/glad.h>

#define VAO_QUAD 0
#define VAOS_MAX 1

extern GLuint vaos[VAOS_MAX];

extern void vaos_init();
extern void vaos_deinit();

#endif
