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

extern void kge_timer_wait(struct kge_timer *timer, uint64_t ns)
{
    struct timespec current;
    if (clock_gettime(CLOCK_MONOTONIC, &current) == -1) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
    }
    struct timespec complete_time = { ns / BILLION, ns % BILLION };
    tsadd(&complete_time, &timer->start);
    timer->start = complete_time; // Start the timer here for next time
    if (tscmp(&complete_time, &current) <= 0) {
        kprint("Time passed, return immediately");
        return;
    }
    tssub(&complete_time, &current); // Get time until complete time
    nanosleep(&complete_time, NULL); // Sleep that long
}

extern void kge_timer_start(struct kge_timer *timer) {
    if (clock_gettime(CLOCK_MONOTONIC, &timer->start) == -1) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
    }
}

extern uint64_t kge_timer_interval(struct kge_timer *timer) {
    struct timespec current;
    if (clock_gettime(CLOCK_MONOTONIC, &current) == -1) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
    }
    tssub(&current, &timer->start);
    return current.tv_sec * BILLION + current.tv_nsec;
}

extern uint64_t kge_timer_reset(struct kge_timer *timer) {
    struct timespec current;
    if (clock_gettime(CLOCK_MONOTONIC, &current) == -1) {
        kprint("Error reading monotonic clock: %s", strerror(errno));
    }
    struct timespec duration = current;
    tssub(&duration, &timer->start);
    timer->start = current;
    return duration.tv_sec * BILLION + duration.tv_nsec;
}
