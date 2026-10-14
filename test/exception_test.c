#include "stdio.h"
#include "exception_test.h"

void trigger_divide_by_zero(){
    printf("[TRIGGER] divide by zero exception");
    int x = 5/0;
}

void trigger_invalid_opcode(){
    printf("[TRIGGER] Invalid opcode (UD2)\n");
    asm volatile ("ud2");
}

void trigger_general_protection_fault(){
    printf("[TRIGGER] General protection fault\n");
    __asm__ volatile("mov $0xFFFF, %ax; mov %ax, %ds");
}

void trigger_page_fault(){
    printf("[TRIGGER] page fault\n");
    int* x = (int*)0x80000000;
    *x = 9;
}