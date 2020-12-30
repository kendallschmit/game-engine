#ifndef KGE_TIMER_H
#define KGE_TIMER_H

#include <stdint.h>
#include <time.h>

void kge_timer_now(struct timespec *ts);
uint64_t kge_timer_nanos_diff(struct timespec *a, struct timespec *b);

#endif
