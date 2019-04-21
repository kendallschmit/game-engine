#include "kutil.h"

static char const *kprint_last_file = NULL;
static char const *kprint_last_func = NULL;
extern bool kutil_kprint_needs_header(char const *file, char const *func)
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
