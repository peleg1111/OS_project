#include "tss.h"
#include "stdio.h"
#include "processe.h"
#include "thread.h"
#include "memory.h"

Tss_entry tss;
gdt_entry_t gdt[6];

gdt_ptr_t gdt_ptr;

extern void create_gdt();

void tss_init(unsigned short kernel_data_segment , unsigned int kernel_esp)
{
    init_gdt();

    memset(&tss, 0, sizeof(Tss_entry));
    
    tss.prev_tss = null;
    tss.ss0 = kernel_data_segment;
    tss.esp0 = kernel_esp;
    tss.iomap_base = sizeof(Tss_entry);

    unsigned int base = (unsigned int)&tss;
    unsigned int limit = sizeof(Tss_entry) - 1;

    gdt_set_gate(5, base, limit, 0x89, 0x00);

    __asm__ __volatile__("ltr %%ax" ::"a"(0x28));

}

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);

    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;

    gdt[num].access = access;
}

void init_gdt()
{
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base = (unsigned int)&gdt;

    // Null Segment
    gdt_set_gate(0, 0, 0, 0, 0);

    // Kernel Code Segment (0x08): Base=0, Limit=4GB, DPL=0
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Kernel Data Segment (0x10): Base=0, Limit=4GB, DPL=0
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // User Code Segment (0x18/0x1B): Base=0, Limit=4GB, DPL=3
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // User Data Segment (0x20/0x23): Base=0, Limit=4GB, DPL=3
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);


    create_gdt();
}