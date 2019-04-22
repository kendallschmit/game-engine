#include "ktiming.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>

#include "kutil.h"

#define MAX_FRAMERATE 240

#define BILLION 1000000000

static struct timespec last_frame_time;
static struct timespec next_frame_time;
static struct timespec frame_time = {
    .tv_sec = 0,
    .tv_nsec = BILLION / MAX_FRAMERATE,
};

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
        a->tv_sec += 1;
        a->tv_nsec -= BILLION;
    }
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

extern void ktiming_frame_started() {
    if (clock_gettime(CLOCK_MONOTONIC, &last_frame_time) == -1) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
    }
    next_frame_time = last_frame_time;
    tsadd(&next_frame_time, &frame_time);
}

extern void ktiming_wait()
{
    // Current time
    struct timespec current;
    if (clock_gettime(CLOCK_MONOTONIC, &current) == -1) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
    }
    // Sleep if we would exceed framerate limit
    if (tscmp(&current, &next_frame_time) <= 0) {
        struct timespec duration = next_frame_time;
        tssub(&duration, &current);
        nanosleep(&duration, NULL);
    }
}

extern void ktiming_starttimer(struct ktiming_timer *timer) {
    if (clock_gettime(CLOCK_MONOTONIC, &timer->start) == -1) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
    }
}

extern double ktiming_endtimer(struct ktiming_timer *timer) {
    struct timespec current;
    if (clock_gettime(CLOCK_MONOTONIC, &current) == -1) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
    }
    struct timespec duration = current;
    tssub(&duration, &timer->start);
    timer->start = current;
    return (double)duration.tv_sec + (double)duration.tv_nsec / BILLION;
}
