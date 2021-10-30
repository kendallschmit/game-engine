#include "kge_timer.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>

#include "kge_util.h"

uint64_t kge_timer_now(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts)) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
        return 0;
    }
    uint64_t nanoseconds = ts.tv_nsec;
    nanoseconds += ts.tv_sec * 1000000000;
    return nanoseconds;
}
