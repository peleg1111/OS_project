#ifndef PHYSICAL_MEMORY_H
#define PHYSICAL_MEMORY_H

unsigned int alloc_frame();
int is_used(unsigned int frame_number);
void set_bitmap(unsigned int frame_number, int turn_on);
void physical_memory_init();


#endif