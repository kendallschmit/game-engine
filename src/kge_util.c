#include "kge_util.h"

#include <stdlib.h>
#include <pthread.h>

static char const *kprint_last_file = NULL;
static char const *kprint_last_func = NULL;
bool kge_util_kprint_needs_header(char const *file, char const *func)
{
    if (kprint_last_file != file || kprint_last_func != func) {
        kprint_last_file = file;
        kprint_last_func = func;
        return true;
    }
    kprint_last_file = file;
    kprint_last_func = func;
    return false;
}

GLfloat randf(GLfloat minimum, GLfloat maximum) {
    GLfloat r = (GLfloat)rand() / RAND_MAX;
    r *= maximum - minimum;
    return r += minimum;
}

GLint randi(GLfloat minimum, GLfloat maximum) {
    GLfloat r = (GLfloat)rand() / RAND_MAX;
    r *= maximum - minimum;
    return r += minimum;
}


