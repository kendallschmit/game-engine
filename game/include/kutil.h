#ifndef KUTIL_H
#define KUTIL_H

#include <stdio.h>

#define kprint(...) do {\
    printf("%s::%s(): ", __FILE__, __func__);\
    printf(__VA_ARGS__);\
    printf("\n");\
    fflush(stdout);\
} while (0);

#endif
