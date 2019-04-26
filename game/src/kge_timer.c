#include "kge_timer.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>

#include "kge_util.h"

#define BILLION 1000000000

static int tscmp(struct timespec *a, struct timespec *b)
{
    if (a->tv_sec > b->tv_sec)
        return 1;
    if (a->tv_sec < b->tv_sec)
        return -1;
    if (a->tv_nsec > b->tv_nsec)
        return 1;
    if (a->tv_nsec < b->tv_nsec)
        return -1;
    return 0;
}

static void tsadd(struct timespec *a, struct timespec *b)
{
    a->tv_sec += b->tv_sec;
    a->tv_nsec += b->tv_nsec;
    while (a->tv_nsec >= BILLION) {
        a->tv_sec += 1; a->tv_nsec -= BILLION; }
}

static void tssub(struct timespec *a, struct timespec *b)
{
    a->tv_sec -= b->tv_sec;
    if (a->tv_nsec < b->tv_nsec) {
        a->tv_sec -= 1;
        a->tv_nsec += BILLION;
    }
    a->tv_nsec -= b->tv_nsec;
}

extern void kge_timer_now(struct timespec *ts)
{
    if (clock_gettime(CLOCK_MONOTONIC, ts)) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
    }
}

extern uint64_t kge_timer_nanos_diff(struct timespec *a, struct timespec *b)
{
    uint64_t nanos = (a->tv_sec - b->tv_sec) * BILLION;
    nanos += a->tv_nsec - b->tv_nsec;
    return nanos;
}
