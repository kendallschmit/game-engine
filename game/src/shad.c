#include "shad.h"

#include <stdlib.h>
#include <stdio.h>

#include "kutil.h"

// Should make this dynamic or something
#define SHADER_SOURCE_LEN 4096

GLuint shad_simple_prog = 0;

static GLuint load_shader(char *path, GLenum shader_type) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        kprint("Unable to open shader file \"%s\"", path);
        return 0;
    }

    GLchar source[SHADER_SOURCE_LEN] = { 0 };
    GLchar *lines = source;
    const GLchar **lines_ptr = &lines;
    if (fread(source, sizeof(source[0]), SHADER_SOURCE_LEN, f) == 0) {
        kprint("Problem reading shader file \"%s\"", path);
        return 0;
    }
    GLint success;
    GLuint shad = glCreateShader(shader_type);
    glShaderSource(shad, 1, lines_ptr, NULL);
    glCompileShader(shad);
    glGetShaderiv(shad, GL_COMPILE_STATUS, &success);
    if (success) {
        kprint("Compiled \"%s\"", path);
    }
    else {
        GLint log_size = 0;
        glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &log_size);
        GLchar log[log_size];
        glGetShaderInfoLog(shad, log_size, NULL, log);
        kprint("Failed to compile \"%s\": %s", path, log);
    }
    return shad;
}

static GLuint fshad = 0;
static GLuint vshad = 0;

extern void shad_init() {
    vshad = load_shader("res/shader/simple_vertex.glsl", GL_VERTEX_SHADER);
    fshad = load_shader("res/shader/simple_fragment.glsl", GL_FRAGMENT_SHADER);

    // Make a basic program, attach shaders
    shad_simple_prog = glCreateProgram();
    glAttachShader(shad_simple_prog, vshad);
    glAttachShader(shad_simple_prog, fshad);

    glLinkProgram(shad_simple_prog);
}

extern void shad_deinit() {
    glDeleteShader(vshad);
    glDeleteShader(fshad);

    glDetachShader(shad_simple_prog, vshad);
    glDetachShader(shad_simple_prog, fshad);
    glDeleteProgram(shad_simple_prog);
}
