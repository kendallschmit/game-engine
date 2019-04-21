#ifndef KUTIL_H
#define KUTIL_H

#include <stdio.h>

#ifndef KMODULE
#define KMODULE "unknown"
#endif

#define kprint(...) do {\
    printf("%s::%s(): ", KMODULE, __func__);\
    printf(__VA_ARGS__);\
    printf("\n");\
    fflush(stdout);\
} while (0);

#endif
