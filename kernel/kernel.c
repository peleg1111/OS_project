#include "const.h"
#include "idt.h"
#include "key_board.h"


void main() {
    
    clear_screen();

    // הדפסת הודעה ראשונה דרך הדרייבר
    printf("Driver System: Loaded Successfully.\n");
    printf("Memory Layout: 32-bit Protected Mode Active.\n");
    printf("Kernel Status: Stable.\n");

    load_idt();
    pic_remap();
    //  חסום Interrupt ה  cli לאחר הפקודה  
    __asm__ __volatile__("sti");// Interrupt מאפשר את ה 
    header_msg();
    while(1){
        __asm__ __volatile__("hlt");
    }
    
}