#include "VM_area.h"
#include "stdio.h"
#include "memory.h"
#include "idt.h"
#include "thread.h"

VM_area *vma_list = null;

VM_area* find_vma( VM_area* vma ,unsigned int addr){
    VM_area *current = vma;
 
    while (current != null)
    {
        if(addr >= current->start && addr < current->end){
            return current;
        }
        current = current->next;
    }
    return null;
    
}

void print_vma_list(VM_area* vma){
    VM_area *current = vma;
    if (current == null)
    {
        return;
    }
    while (current != null){
        printf("\n");
        if(current->type != null){
            printf("vma name:%s",current->type);
        }
        printf("\tstart:%p\tend:%p\n", current->start, current->end);

        unsigned int flags = current->flags;
        printf("flags:%p -->",flags);

        if (flags & 0x1)
            printf(" read");
        if (flags & 0x2)
            printf(" ,write");
        if (flags & 0x4)
            printf(" ,execute");
        if (flags & 0x8)
            printf(" ,kernel");

        printf("\n");
        current = current -> next;
    }
}


int add_vma(VM_area **vma_head_ptr, unsigned int start, unsigned int end, unsigned int flags, char type[])
{
    if (vma_head_ptr == null)
        return 0;

    VM_area *current = *vma_head_ptr;
    VM_area *last = null;

    while (current != null)
    {
        // בדיקת חפיפה
        if (start < current->end && end > current->start)
        {
            return 0; // יש חפיפה
        }
        last = current;
        current = current->next;
    }

    VM_area *new_vma = kmalloc(sizeof(VM_area));
    if (new_vma == null)
        return 0;

    new_vma->start = start;
    new_vma->end = end;
    new_vma->flags = flags;
    new_vma->type = type;
    new_vma->next = null;

    if (*vma_head_ptr == null)
    {
        *vma_head_ptr = new_vma; // עדכון ראש הרשימה המקורי
    }
    else
    {
        last->next = new_vma;
    }
    return 1;
}

void page_fault_handler(registers_t *regs)
{
    unsigned int addr;
    __asm__ volatile("mov %%cr2, %0" : "=r"(addr));

    int present = !(regs->err_code & 0x1); // דף לא ממופה <- 0
    int write = regs->err_code & 0x2;      // 1 -> ניסיון כתיבה
    int user = regs->err_code & 0x4;       // 1 -> User Mode (Ring 3)
    int reserved = regs->err_code & 0x8;   // 1 -> דריסת ביטים שמורים
    int fetch = regs->err_code & 0x10;     // 1 -> ניסיון להריץ קוד

    if(present){
        if (!th_list || !th_list->thread || !th_list->thread->processe || !th_list->thread->processe->vma)
        {
            printf("\n[PANIC] Segmentation Fault at %p! Invalid process context.\n", addr);
            error_print(regs);
            return;
        }

        VM_area *vma = find_vma(th_list->thread->processe->vma, addr);
        if (vma != null)
        {
            unsigned int page_virtual_addr = addr & 0xFFFFF000;
            unsigned int is_user = (vma->flags & 0x8) ? 0 : 1;

            if (!map_page(page_virtual_addr, is_user))
            {
                printf("\n[PANIC] Out of memory mapping %p\n", addr);
                error_print(regs);
            }
            else{
                return;
            }
        }
        else
        {
            print_vma_list(th_list->thread->processe->vma);
            printf("\nSegmentation Fault at %p! No VMA found.\n", addr);
            error_print(regs);
        }
    }
    printf("\n================ [ PAGE FAULT ] ================\n");
    printf("Faulting Address (CR2): %x\n", addr);
    printf("Instruction Pointer (EIP): %x\n", regs->eip);
    printf("Error Code: %x [ %s | %s | %s ]\n",
           regs->err_code,
           present ? "Page Protection Violation" : "Page Not Present",
           write ? "Write Access" : "Read Access",
           user ? "User Mode (Ring 3)" : "Kernel Mode (Ring 0)");

    if (reserved)
        printf(" -> Cause: Reserved bit overwritten\n");
    if (fetch)
        printf(" -> Cause: Instruction Fetch Exception\n");
    printf("=================================================\n");

    if (user)
    {
        printf("Kernel: Terminating process due to SegFault.\n");
        set_current_as_dead();
        yield;
    }
    else
    {
        // Kernel Mode-שגיאה ב 
        printf("Kernel Panic: Page Fault in Kernel Mode!\n");
        while (1)
        {
            __asm__ __volatile__("cli; hlt");
        }
    }
}