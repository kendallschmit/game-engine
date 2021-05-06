#include "shader.h"

#include "kge_util.h"

// Should make this dynamic or something
#define SHADER_SOURCE_LEN 4096

GLuint shader_program_simple = 0;
GLuint shader_program_line = 0;

static GLuint load_shader(char *path, GLenum shader_type) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        kprint("Unable to open shader file \"%s\"", path);
        return 0;
    }
    // Simulate the array of lines that glShaderSource expects
    GLchar source[SHADER_SOURCE_LEN] = { 0 };
    GLchar *lines = source;
    const GLchar **lines_ptr = (const GLchar **)&lines;
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

// Fragment shaders
static GLuint fragment_simple_uv = 0;
static GLuint fragment_simple_color = 0;

static GLuint vertex_simple_instanced = 0;
static GLuint vertex_line = 0;

void shader_init() {
    fragment_simple_uv = load_shader("res/shader/simple_fragment.glsl",
            GL_FRAGMENT_SHADER);
    fragment_simple_color = load_shader("res/shader/simple_color.glsl",
            GL_FRAGMENT_SHADER);

    vertex_simple_instanced = load_shader("res/shader/simple_vertex.glsl",
            GL_VERTEX_SHADER);
    vertex_line = load_shader("res/shader/simple_line.glsl",
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
