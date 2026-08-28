#include "timer.h"
#include "stdio.h"
#include "idt.h"

void update_time();
void print_char_at(volatile unsigned short *video, int offset, char c);
void read_RTC();
void sleep(float sec);
unsigned int Time();
char *get_day_string(int day);

volatile timer_t time = {
    .sec = 0,
    .tick_count = 0,
    .miniute_count = 0,
    .hour_count = 0,
    .day_count = 0,
    .frequency = 0,
    .year_count = 0,
    .century_count = 0,
    .month_count = 0,
    .day_of_week = 0,
    .sleep = null,
    .time = null
};

void timer_handler()
{
    port_byte_out(0x20, 0x20);
    time.tick_count ++;

    if(time.tick_count % (time.frequency / 2) == 0){
        read_RTC();
        update_time();
    }
    if(time.tick_count % 3 == 0)
    {
        yield;
    }
}

void init_timer(unsigned int freq)
{
    print_info.text_offset += 1;
    volatile unsigned short *video = print_info.video;

    time.sleep = sleep;
    time.time = Time;



    for (int i = 0; i < SCREEN_COLS; i++)
    {
        print_char_at(video, i, ' ');
    }

    time.frequency = freq;
    unsigned int divisor = 1193182 / freq;

    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, (unsigned char)(divisor & 0xFF));
    port_byte_out(0x40, (unsigned char)((divisor >> 8) & 0xFF));
    
    read_RTC();
    update_time();
}

void print_num_at(volatile unsigned short *video, int offset, int num)
{
    char tens = (num / 10) + '0';
    char ones = (num % 10) + '0';

    video[offset] = tens | (0x0F << 8);
    video[offset + 1] = ones | (0x0F << 8);
}

void print_char_at(volatile unsigned short *video, int offset, char c)
{
    video[offset] = c | (0x0F << 8);
}


void print_string_at(volatile unsigned short *video, int offset, char str[])
{
    for(int i = 0; str[i] != null; i++)
    {
        video[offset + i] = str[i] | (0x0F << 8);
    }
}


void update_time()
{
    volatile unsigned short *video = print_info.video;

    int start_offset = SCREEN_COLS / 2 - 17;
    print_num_at(video, start_offset, time.day_count);
    print_char_at(video, start_offset + 2, '/');
    print_num_at(video, start_offset + 3, time.month_count);
    print_char_at(video, start_offset + 5, '/');
    print_num_at(video, start_offset + 6, time.century_count);
    print_num_at(video, start_offset + 8, time.year_count);


    print_char_at(video, start_offset + 10, ' ');
    print_num_at(video, start_offset + 11, time.hour_count);
    print_char_at(video, start_offset + 13, ':');
    print_num_at(video, start_offset + 14, time.miniute_count);
    print_char_at(video, start_offset + 16, ':');
    print_num_at(video, start_offset + 17, time.sec);
    print_char_at(video, start_offset + 19, ' ');
    print_char_at(video, start_offset + 20, ' ');
    print_char_at(video, start_offset + 21, 'd');
    print_char_at(video, start_offset + 22, 'a');
    print_char_at(video, start_offset + 23, 'y');
    print_char_at(video, start_offset + 24, ':');
    print_char_at(video, start_offset + 25, ' ');

    print_string_at(video, start_offset + 26, get_day_string(time.day_of_week) );
}

void sleep(float sec)
{
    unsigned long target_tick = time.tick_count + sec * time.frequency;

    while (time.tick_count < target_tick)
    {
        __asm__ volatile("sti");
        __asm__ volatile("hlt");
    }
}

void read_RTC(){
    int res;
    do{
        port_byte_out(0x70, 0x0A);
        res = port_byte_in(0x71);

    } while(( res & 0x80 ) != 0);

    port_byte_out(0x70,0x00);
    int sec = port_byte_in(0x71);
    sec = 10*(sec/16) + (sec & 0x0F);

    port_byte_out(0x70, 0x02);
    int miniutes = port_byte_in(0x71);
    miniutes = 10 * (miniutes / 16) + (miniutes & 0x0F);

    port_byte_out(0x70, 0x04);
    int hours = port_byte_in(0x71);
    hours = 10 * (hours / 16) + (hours & 0x0F);

    hours = (hours + 3) % 24; // תיקון איזור זמן(הזמן של לוח האם לא הולך לפי שעון ישראל)

    port_byte_out(0x70, 0x07);
    int days = port_byte_in(0x71);
    days = 10 * (days / 16) + (days & 0x0F);

    port_byte_out(0x70, 0x08);
    int month = port_byte_in(0x71);
    month = 10 * (month / 16) + (month & 0x0F);

    port_byte_out(0x70, 0x09);
    int year = port_byte_in(0x71);
    year = 10 * (year / 16) + (year & 0x0F);

    port_byte_out(0x70, 0x32);
    int century = port_byte_in(0x71);
    century = 10 * (century / 16) + (century & 0x0F);

    port_byte_out(0x70, 0x06);
    int day_of_week = port_byte_in(0x71);
    

    time.sec = sec;
    time.miniute_count = miniutes;
    time.hour_count = hours;
    time.day_count = days;
    time.year_count = year;
    time.century_count = century;
    time.month_count = month;
    time.day_of_week = day_of_week;
}


unsigned int Time(){
    return time.tick_count;
}

char *get_day_string(int day)
{
    switch (day)
    {
    case 1:
        return "Sunday";
    case 2:
        return "Monday";
    case 3:
        return "Tuesday";
    case 4:
        return "Wednesday";
    case 5:
        return "Thursday";
    case 6:
        return "Friday";
    case 7:
        return "Saturday";
    default:
        return "Unknown";
    }
}