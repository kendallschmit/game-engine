#include "shad.h"

#include <stdlib.h>
#include <stdio.h>

// Shader literals
#define MY_VERTEX_SHADER "#version 410\n\
uniform mat4 projmat;\n\
uniform mat4 viewmat;\n\
uniform mat4 modelmat;\n\
layout (location = 0) in vec3 in_pos;\n\
layout (location = 1) in vec2 vert_uv;\n\
out vec2 uv;\n\
void main(void) {\n\
    gl_Position = projmat * viewmat * modelmat * vec4(in_pos.xyz, 1.0);\n\
    uv = vert_uv;\n\
}\
"

#define MY_FRAGMENT_SHADER "#version 410\n\
uniform sampler2D tex_sampler;\n\
in vec2 uv;\n\
out vec4 out_color;\n\
void main (void) {\n\
    out_color = texture(tex_sampler, uv).rgba;\n\
}\
"

GLuint shad_simple_prog = 0;

static GLuint compile_shader(const GLchar * const *source, GLenum shader_type) {
    printf("--------\n%s\n... ", *source);
    GLint success;

    GLuint shad = glCreateShader(shader_type);
    glShaderSource(shad, 1, source, NULL);
    glCompileShader(shad);

    glGetShaderiv(shad, GL_COMPILE_STATUS, &success);
    if (success) {
        printf("compiled\n");
    }
    else {
        printf("failed\n");
    }
    printf("log:\n");
    GLint log_size = 0;
    glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &log_size);
    GLchar log[log_size];
    glGetShaderInfoLog(shad, log_size, NULL, log);
    printf("%s\n--------\n", log);

    return shad;
}

static GLuint fshad = 0;
static GLuint vshad = 0;

extern void shad_init() {
    printf("begin shad_init()\n");
    // Make basic shaders
    const GLchar *line;
    const GLchar *const *source;

    line = MY_VERTEX_SHADER;
    source = &line;
    vshad = compile_shader(source, GL_VERTEX_SHADER);

    line = MY_FRAGMENT_SHADER;
    source = &line;
    fshad = compile_shader(source, GL_FRAGMENT_SHADER);

    // Make a basic program, attach shaders
    shad_simple_prog = glCreateProgram();
    glAttachShader(shad_simple_prog, vshad);
    glAttachShader(shad_simple_prog, fshad);

    glLinkProgram(shad_simple_prog);

    printf("end shad_init()\n\n");
}

extern void shad_deinit() {
    glDeleteShader(vshad);
    glDeleteShader(fshad);

    glDetachShader(shad_simple_prog, vshad);
    glDetachShader(shad_simple_prog, fshad);
    glDeleteProgram(shad_simple_prog);
}
