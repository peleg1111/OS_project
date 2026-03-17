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
    pic_remap();
    load_idt();
    __asm__ __volatile__("sti");

    terminal_init();
    
    printf("kernel loaded successfully");
    header_msg();
}