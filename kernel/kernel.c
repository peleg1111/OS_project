#include "const.h"
#include "idt.h"
#include "key_board.h"
void set_up_kernel();

void main() {
    
    set_up_kernel();

    while(1){
        __asm__ __volatile__("hlt");
    }
    
}

//  מאפס את ההגדרות של הקרנל ומכין אותו לשימוש
void set_up_kernel(){
    
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
}