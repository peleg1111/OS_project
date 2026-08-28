#ifndef VM_AREA_H
#define VM_AREA_H

#include "idt.h"

typedef struct VM_area
{
    unsigned int start;
    unsigned int end;
    unsigned int flags : 4 ;//הרשאות(קריאה, כתיבה, הרצה, קרנל)
    char* type;// עבור דיבאג -> מציין מי השתמש באיזור הזיכרון הזה)
    struct VM_area* next;
    
}__attribute__((packed)) VM_area;



extern VM_area* vma_list;

int add_vma(VM_area **vma ,unsigned int start, unsigned int end, unsigned int flags, char type[]);
void print_vma_list(VM_area *vma);

void page_fault_handler(registers_t* regs);

VM_area *find_vma(VM_area *vma ,unsigned int addr);

#endif