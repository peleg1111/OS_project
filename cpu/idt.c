#include "idt.h"

idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

extern void isr_wrapper(); // מגיע מה-ASM

void set_idt_gate(int n, unsigned int handler) {
    idt[n].low_offset = (handler & 0xFFFF);
    idt[n].sel = 0x08; // הקוד סגמנט שלנו ב-GDT
    idt[n].always0 = 0;
    idt[n].flags = 0x8E; // 10001110: שער פסיקה, רמת הרשאה 0 (Kernel)
    idt[n].high_offset = (handler >> 16) & 0xFFFF;
}

void load_idt() {

    set_idt_gate(33, (unsigned int)isr_wrapper);
    
    idt_reg.base = (unsigned int)&idt;
    idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
    // פקודת Assembly שטוענת את הכתובת למעבד
    __asm__ __volatile__("lidt (%0)" : : "r" (&idt_reg));
}