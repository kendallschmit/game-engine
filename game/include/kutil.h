#ifndef KUTIL_H
#define KUTIL_H

#include <stdio.h>
#include <stdbool.h>

extern bool kutil_kprint_needs_header(char const *file, char const *func);

#define kprint(...) do {\
    if (kutil_kprint_needs_header(__FILE__, __func__)) {\
        printf("%s::%s():\n", __FILE__, __func__);\
    }\
    printf(" * ");\
    printf(__VA_ARGS__);\
    printf("\n");\
    fflush(stdout);\
} while (0);

#endif
