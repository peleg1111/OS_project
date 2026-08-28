#include "stdio.h"
#include "idt.h"
#include "key_board.h"
#include "timer.h"
#include "memory.h"
#include "VM_area.h"
#include "thread.h"

void set_up_kernel();
void pic_remap();
void test3();

void main()
{
    set_up_kernel();

    start_thread(create_processe(test3));

    while(1){

        //time.sleep(2);
        //printf("%d\t",time.tick_count);
        
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

    init_timer(180);
    init_threading();

    header_msg();

    __asm__ __volatile__("sti"); // פתיחת פסיקות
}

void pic_remap()
{
    port_byte_out(0x20, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0x21, 0x04);
    port_byte_out(0x21, 0x01);

    port_byte_out(0xA0, 0x11);
    port_byte_out(0xA1, 0x28);
    port_byte_out(0xA1, 0x02);
    port_byte_out(0xA1, 0x01);


    // 0xF8 = 11111000 -> פותח את IRQ0 (טיימר), IRQ1 (מקלדת), IRQ2 (Cascade לסלייב)
    port_byte_out(0x21, 0xF8);

    // 0xEF = 11101111 -> פותח את IRQ12 (עכבר) בתוך ה-Slave PIC
    port_byte_out(0xA1, 0xEF);
}

int sum = 0;

void test(){
    for (int i = 0; i < 80; i++)
    {
        time.sleep(0.1/2);
        sum++;
        printf("\nsum = %d", sum);  
    }
}

void test2(){

    for (int i = 0; i < 80; i++)
    {
        time.sleep(0.1/2);
        sum--;
        printf("\nsum = %d", sum);
    }
}

void test3()
{
    time.sleep(2);
    Thread *t2 = create_thread(test);
    Thread* t3 = create_thread(test);
    Thread *t1 = create_thread(test2);
    start_thread(t1);
    start_thread(t2);
    start_thread(t3);


    join_thread(t1);
    join_thread(t2);
    join_thread(t3);

    printf("\nt3 end");
    header_msg();
}




