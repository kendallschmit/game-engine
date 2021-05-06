#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

extern GLuint shader_program_simple;
extern GLuint shader_program_line;

void shader_init();
void shader_deinit();

#endif
