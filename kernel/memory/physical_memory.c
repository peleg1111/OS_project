#include "physical_memory.h"
#include "stdio.h" 
#include "memory.h"

unsigned int alloc_frame();
int is_used(unsigned int frame_number);
void set_bitmap(unsigned int frame_number, int turn_on);
void physical_memory_init();
unsigned int memory_bitmap[memory_bitmap_size];//כל ביט מייצג פריים שלם
extern int kernel_end;



void physical_memory_init()
{
    for(int i = 0; i < sizeof(memory_bitmap) / sizeof(memory_bitmap[0]) ; i++){
        memory_bitmap[i] = 0;
    }

    unsigned int pages_to_map = kernel_limit / (kb1 * frame_size);

    unsigned int frame_num = (pages_to_map + 1) * kb1;

    
    for (unsigned int i = 0; i < frame_num; i++)
    {
        set_bitmap(i, 1);
    }
}

void set_bitmap(unsigned int frame_number, int turn_on)
{
    int index = frame_number / 32;
    int offset = frame_number % 32;

    if (index >= memory_bitmap_size) return;
    
    if (turn_on)
    {
        memory_bitmap[index] |= (1 << offset);
    }
    else
    {
        memory_bitmap[index] &= ~(1 << offset);
    }
}

int is_used(unsigned int frame_number)
{
    int index = frame_number / 32;
    int offset = frame_number % 32;
    if (index >= memory_bitmap_size)
    {
        return 1;
    }
    return (memory_bitmap[index] & (1 << offset)) != 0;
}

unsigned int alloc_frame()
{

    for (unsigned int i = 0; i < memory_bitmap_size * 32; i++)
    {
        if (!is_used(i))
        {
            set_bitmap(i, 1);
            unsigned int phys_addr = i * 0x1000;
            //printf("\nalloc frame %d, phys_addr = %x", i, phys_addr);
            return phys_addr;
        }
    }

    return null;
}


int get_free_frames_count()
{
    int count = 0;
    for (unsigned int i = 0; i < memory_bitmap_size * 32; i++)
    {
        if (!is_used(i))
        {
            count++;
        }
    }
    return count;
}
