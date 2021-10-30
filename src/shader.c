#include "shader.h"

#include <string.h>

#include "kge_util.h"
#include "res_shader.h"

// Should make this dynamic or something
#define SHADER_SOURCE_LEN 4096

GLuint shader_program_simple = 0;
GLuint shader_program_line = 0;

static GLuint load_shader(uint8_t *buf, size_t len, GLenum shader_type) {
    kprint("Loading shader...");
    GLchar lines[len + 1];
    for (size_t i = 0; i < len; i++) {
        lines[i] = (GLchar)buf[i];
    }
    lines[len] = 0;
    const GLchar *lines_ptr = lines;
    GLint success;
    GLuint shad = glCreateShader(shader_type);
    glShaderSource(shad, 1, &lines_ptr, NULL);
    glCompileShader(shad);
    glGetShaderiv(shad, GL_COMPILE_STATUS, &success);
    if (success) {
        kprint("Compiled shader");
    }
    else {
        GLint log_size = 0;
        glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &log_size);
        GLchar log[log_size];
        glGetShaderInfoLog(shad, log_size, NULL, log);
        kprint("Failed to compile shader: %s", log);
    }
    return shad;
}

// Fragment shaders
static GLuint fragment_simple_uv = 0;
static GLuint fragment_simple_color = 0;

static GLuint vertex_simple_instanced = 0;
static GLuint vertex_line = 0;

void shader_init() {
    fragment_simple_uv = load_shader(res_shader_simple_fragment,
            sizeof(res_shader_simple_fragment),
            GL_FRAGMENT_SHADER);
    fragment_simple_color = load_shader(res_shader_simple_color,
            sizeof(res_shader_simple_color),
            GL_FRAGMENT_SHADER);

    vertex_simple_instanced = load_shader(res_shader_simple_vertex,
            sizeof(res_shader_simple_vertex),
            GL_VERTEX_SHADER);
    vertex_line = load_shader(res_shader_simple_line,
            sizeof(res_shader_simple_line),
            GL_VERTEX_SHADER);

    // Program for vertex shader
    shader_program_simple = glCreateProgram();
    glAttachShader(shader_program_simple, vertex_simple_instanced);
    glAttachShader(shader_program_simple, fragment_simple_uv);
    glLinkProgram(shader_program_simple);

    // Program for line shader
    shader_program_line = glCreateProgram();
    glAttachShader(shader_program_line, vertex_line);
    glAttachShader(shader_program_line, fragment_simple_color);
    glLinkProgram(shader_program_line);

    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        kprint("GL Error: 0x%x", (int)error);
    }
}

void shader_deinit() {
    glDeleteShader(fragment_simple_uv);
    glDeleteShader(fragment_simple_color);
    glDeleteShader(vertex_simple_instanced);
    glDeleteShader(vertex_line);

    glDetachShader(shader_program_simple, vertex_simple_instanced);
    glDetachShader(shader_program_simple, fragment_simple_uv);
    glDeleteProgram(shader_program_simple);

    glDetachShader(shader_program_line, vertex_line);
    glDetachShader(shader_program_line, fragment_simple_color);
    glDeleteProgram(shader_program_line);
}
