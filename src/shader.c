#include "shader.h"

#include "kge_util.h"

// Should make this dynamic or something
#define SHADER_SOURCE_LEN 4096

GLuint shader_program_simple = 0;

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

static GLuint fshad = 0;
static GLuint vshad = 0;

extern void shader_init() {
    vshad = load_shader("res/shader/simple_vertex.glsl", GL_VERTEX_SHADER);
    fshad = load_shader("res/shader/simple_fragment.glsl", GL_FRAGMENT_SHADER);

    // Make a basic program, attach shaders
    shader_program_simple = glCreateProgram();
    glAttachShader(shader_program_simple, vshad);
    glAttachShader(shader_program_simple, fshad);

    glLinkProgram(shader_program_simple);
}

extern void shader_deinit() {
    glDeleteShader(vshad);
    glDeleteShader(fshad);

    glDetachShader(shader_program_simple, vshad);
    glDetachShader(shader_program_simple, fshad);
    glDeleteProgram(shader_program_simple);
}
