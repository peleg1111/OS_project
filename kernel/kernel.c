#include "stdio.h"
#include "idt.h"
#include "key_board.h"
#include "timer.h"
#include "memory.h"
#include "VM_area.h"
#include "thread.h"
#include "process_test.h"

void set_up_kernel();

void main()
{

    set_up_kernel();
    
    unsigned int s = calc_heap_size(th_list->thread->process->heap);

    int frames = get_free_frames_count();
    Thread* t1 = create_thread(test_process_running_process);
    Thread* t2 = create_processe(test_process_free);

    start_thread(t1);
    start_thread(t2);
    join_thread(t1);
    join_thread(t2);
    for(int i = 0; i< 100; i++) yield;

    printf("\n    size = %d   end size = %d",s,calc_heap_size(th_list->thread->process->heap));
    printf("\n  start frames: %d end frames = %d",frames,get_free_frames_count());
    

    while(1){
        __asm__ __volatile__("hlt");
    }
}

//  מאפס את ההגדרות של הקרנל ומכין אותו לשימוש  
void set_up_kernel() {
    terminal_init();
    clear_screen();

    pic_remap();
    load_idt();

    memory_init();

    init_timer(250);
    init_threading();

    header_msg();

    __asm__ __volatile__("sti"); // פתיחת פסיקות
}



