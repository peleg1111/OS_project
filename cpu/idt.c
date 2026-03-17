#include "idt.h"

idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

extern void isr_wrapper();

void set_idt_gate(int n, unsigned int handler) {
    idt[n].low_offset  = handler & 0xFFFF;
    idt[n].sel         = 0x08;
    idt[n].always0     = 0;
    idt[n].flags       = 0x8E;
    idt[n].high_offset = (handler >> 16) & 0xFFFF;
}

void load_idt() {
    set_idt_gate(33, (unsigned int)isr_wrapper);
    idt_register_t reg;
    reg.base  = (unsigned int)&idt;
    reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;

    __asm__ __volatile__("lidt %0" : : "m" (reg));
    
    printf("IDT limit should be %d\n", reg.limit);
}