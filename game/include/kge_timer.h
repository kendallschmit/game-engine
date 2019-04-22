#ifndef KGE_TIMER_H
#define KGE_TIMER_H

#include <stdint.h>
#include <time.h>

struct kge_timer {
    struct timespec start;
};

extern void kge_timer_wait(struct kge_timer *timer, uint64_t ns);
extern void kge_timer_start(struct kge_timer *timer);
extern uint64_t kge_timer_interval(struct kge_timer *timer);
extern uint64_t kge_timer_reset(struct kge_timer *timer);

#endif
