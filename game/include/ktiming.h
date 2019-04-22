#ifndef KTIMING_H
#define KTIMING_H

#include <stdint.h>
#include <time.h>

#define KTIMING_INPUT 0
#define KTIMING_FRAME 1

extern void ktiming_frame_started();
extern void ktiming_wait();

struct ktiming_timer {
    struct timespec start;
};

extern void ktiming_starttimer(struct ktiming_timer *timer);
extern double ktiming_endtimer(struct ktiming_timer *timer);

#endif
