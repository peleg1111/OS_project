#ifndef PHYSICAL_MEMORY_H
#define PHYSICAL_MEMORY_H

unsigned int alloc_frame();
int is_used(unsigned int frame_number);
void set_bitmap(unsigned int frame_number, int turn_on);
void physical_memory_init();
int get_free_frames_count();
int get_occupied_frames_cout();
void print_memory_map();
#endif