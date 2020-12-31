#ifndef KGE_UTIL_H
#define KGE_UTIL_H

#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof(*a))

bool kge_util_kprint_needs_header(char const *file, char const *func);

#define kprint(...) do {\
    if (kge_util_kprint_needs_header(__FILE__, __func__)) {\
        printf("%s::%s():\n", __FILE__, __func__);\
    }\
    printf(" * ");\
    printf(__VA_ARGS__);\
    printf("\n");\
    fflush(stdout);\
} while (0);

GLfloat randf(GLfloat minimum, GLfloat maximum);
GLint randi(GLfloat minimum, GLfloat maximum);

#endif
