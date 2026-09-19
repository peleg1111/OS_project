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
    start_thread(create_thread(test_process_running_process));
    
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



