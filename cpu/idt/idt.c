#include "idt.h"
#include "terminal_command.h"
#include "stdio.h"
#include "key_board.h"
#include "timer.h"
#include "thread.h"
#include "tss.h"

idt_gate_t idt[IDT_ENTRIES];

extern void isr_wrapper();
extern void divide_by_zero_wrapper();
extern void timer_wrapper();
extern void page_fault_wrapper();
extern void switch_thread_wrapper();
extern void invalid_opcode_wrapper();
extern void double_fault_wrapper();
extern void general_protection_fault_wrapper();


void set_idt_gate(int n, unsigned int handler, unsigned int flags, unsigned int sel) {
    idt[n].low_offset  = handler & 0xFFFF;
    idt[n].sel         = sel;
    idt[n].always0     = 0;
    idt[n].flags       = flags;
    idt[n].high_offset = (handler >> 16) & 0xFFFF;
}

void exception_Division_By_Zero(registers_t *regs)
{
    printf("\t\t\tDivision By Zero");
    error_print(regs); // הדפסת האוגרים
    regs->eip += 2; // איפוס כתובת ההחזרה כדי למנוע חזרה מהפסיקה
    // הקידום של המצביעה צריך להיות דינמי בהתאם לסוג הפסיקה. כאן אני מוסיף 2 בתור דוגמה אבל זה צריך להיות מותאם לכל סוג פסיקה
    header_msg();
    
}

void general_protection_fault_handler(registers_t* regs){

    if(regs->cs == KERNEL_CODE_SEGMENT){
        
        printf("\n\t\t[KERNEL PANIC] general protection fault in kernel mode\n");
        error_print(regs);
        __asm__ volatile("cli");
        for (;;)
        {
            __asm__ volatile("hlt");
        }
    }
    else{
        if(th_list!= null && th_list->thread != null){
            printf("thread id: %d -> general protection fault exception ", th_list->thread->id);
        }
        else{
            printf("thread not exists -> general protection fault exception ");
        }

        set_current_as_dead();
    }
}

void invalid_opcode_handler(registers_t* regs) {
    if((regs->cs & 0x03) == 0){
        printf("\n\t\t[KERNEL PANIC] invalid opcode exception in kernel mode\n");
        error_print(regs);
        __asm__ volatile("cli");
        for (;;)
        {
            __asm__ volatile("hlt");
        }
    }
    else
    {
        if (th_list != null && th_list->thread != null)
        {
            printf("thread id: %d -> invalid opcode exception ", th_list->thread->id);
        }
        else
        {
            printf("thread not exists -> invalid opcode exception ");
        }

        set_current_as_dead();
    }
}

void double_fault_handler(registers_t* regs) {// כדי שזה יעבוד TSS עדיין לא עובד - יש לקשר את זה 

    printf("\n\t\t[KERNEL PANIC]: DOUBLE FAULT\n");

    error_print(regs);
    __asm__ volatile("cli");
    for (;;)
    {
        __asm__ volatile("hlt");
    }
}

void error_print(registers_t* regs) {
    printf("\t\t\tError: An error occurred!");

    printf("\n\tEIP: %p", regs->eip);// כתובת השגיאה
    printf("\tCS: %p\n", regs->cs);// סגמנט הקוד
    printf("\tFlags: %p\n", regs->flags);// דגלי המעבד
    printf("\n\tInterupt Number: %p\n", regs->interupt_number);// מספר הפסיקה
    printf("\tError Code: %p\t", regs->err_code);// קוד השגיאה

    printf("\n\tRegisters:\n");

    printf("\t\tEAX: %p\t", regs->eax);
    printf("\tEBX: %p\n", regs->ebx);
    printf("\t\tECX: %p\t", regs->ecx);
    printf("\tEDX: %p\t", regs->edx);
    printf("\tEBP: %p\n", regs->ebp);
    printf("\t\tESI: %p\t", regs->esi); 
    printf("\tEDI: %p", regs->edi);
}

void load_idt() {

    set_idt_gate(divide_by_zero_port, (unsigned int)divide_by_zero_wrapper, ring0, KERNEL_CODE_SEGMENT); // Divide by zero exception

    set_idt_gate(timer_port, (unsigned int)timer_wrapper, ring0, KERNEL_CODE_SEGMENT); // Timer interrupt

    set_idt_gate(keyboard_port, (unsigned int)isr_wrapper, ring0, KERNEL_CODE_SEGMENT); // Keyboard interrupt

    set_idt_gate(page_fault_port, (unsigned int)page_fault_wrapper, ring0, KERNEL_CODE_SEGMENT); // page fault

    set_idt_gate(switch_thread_port, (unsigned int)switch_thread_wrapper, ring3, KERNEL_CODE_SEGMENT);
    /*
        yield -> int 81 
        מפעיל פסיקה שמחליפה את התהליך לתהליך אחר
    */

    set_idt_gate(invalid_opcode_port, (unsigned int)invalid_opcode_wrapper, ring0, KERNEL_CODE_SEGMENT);// invalid opcode

    //set_idt_gate(double_fault_port, (unsigned int)double_fault_wrapper, ring0, KERNEL_CODE_SEGMENT);// double fault - עדיין לא עובד

    set_idt_gate(general_protection_fault_port, (unsigned int)general_protection_fault_wrapper, ring0, KERNEL_CODE_SEGMENT); // general protection fault


    idt_register_t reg;
    reg.base  = (unsigned int)&idt;
    reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;

    __asm__ __volatile__("lidt %0" : : "m"(reg));
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
