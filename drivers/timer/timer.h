#ifndef TIMER_H
#define TIMER_H 

#include "idt.h"

void timer_handler();
void init_timer(unsigned int frequency);
void sleep( float sec);
void read_RTC();

typedef struct
{
    unsigned int day_of_week;
    unsigned int frequency;
    unsigned int miniute_count;
    unsigned int hour_count;
    unsigned int day_count;
    unsigned int sec;
    unsigned int tick_count;
    unsigned int year_count;
    unsigned int month_count;
    unsigned int century_count; 
    void (*sleep)( float sec);
    unsigned int (*time)();

} timer_t;

extern volatile timer_t time;

#endif