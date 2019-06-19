#ifndef TIMER_H
#define TIMER_H

typedef int timer_status;

enum e_timer_status
{
    timer_idle = 1,
    timer_numara,
    timer_timeout,
};

void            timer_init(void);
void            timer_start(int);
void            timer_stop(void);
timer_status    timer_citeste_status(void);

#endif