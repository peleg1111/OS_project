#include "memory.h"
#include "stdio.h"
#include "physical_memory.h"
#include "VM_area.h"
#include "thread.h"

extern int kernel_end;

Page_entry page_tables[1024][1024] __attribute__((aligned(4096))); 
Page_entry page_directory[1024] __attribute__((aligned(4096)));


Heap_header *user_heap_start_block = null;
unsigned int user_heap_start = (unsigned int)&kernel_end;
unsigned int user_heap_start_addr;


Heap_header *kernel_heap_start_block = null;

unsigned int pde_physical_address = 0;


void paging_init() {
    user_heap_start = kernel_limit;

    user_heap_start_addr = user_heap_start;
    user_heap_start_block = null;
    
    unsigned int kernel_pages_to_map = (kernel_limit) / (kb1 * frame_size);

    for (int i = 0; i < 1024; i++){
        page_directory[i].value = 0;
    }


    for(int i = 0 ;  i < kb1 ; i++){

        page_directory[i].bits.present = 1;
        page_directory[i].bits.read_write = 1;
        page_directory[i].bits.frame = (unsigned int)page_tables[i] >> 12 ;
        page_directory[i].bits.user_supervisor = 1;

        if( i <= kernel_pages_to_map ){            
            for (int j = 0; j < 1024; j++)
            {
                page_tables[i][j].bits.present = 1;
                page_tables[i][j].bits.read_write = 1;
                page_tables[i][j].bits.frame = i * 1024 + j;
                page_tables[i][j].bits.user_supervisor = 0;
            }
        }

        else{
            for (int j = 0; j < 1024; j++)
            {
                page_tables[i][j].value = 0;
            }
        }
    }

    // recursive paging
    page_directory[1023].bits.frame = ((unsigned int)page_directory) >> 12;
    page_directory[1023].bits.present = 1;
    page_directory[1023].bits.read_write = 1;

    pde_physical_address = (unsigned int)page_directory;
    
    __asm__ __volatile__(

        "movl %0, %%eax\n\t"    
        "movl %%eax, %%cr3\n\t"

        "movl %%cr0, %%eax\n\t"
        "orl $0x80000000, %%eax\n\t" // מדליק את ביט 31
        "movl %%eax, %%cr0\n\t"      // Paging הדלקת מנגנון ה

        :                     // אין משתני פלט
        : "r"(page_directory) // משתנה קלט
        : "eax"               // eax נהרס התוכן של 
    );

}



int map_page(unsigned int virtual_addr, unsigned int is_user)
{
    virtual_addr &= 0xFFFFF000;
    
    unsigned int physical_addr = alloc_frame();
    
    if (physical_addr == null){
        return 0;
    }

    unsigned int page_directory_index = virtual_addr >> 22;
    unsigned int page_table_index = (virtual_addr >> 12) & 0x3FF;
    Page_entry* pde = (Page_entry*)pde_addr;
    
    Page_entry* new_table;

    if(pde[page_directory_index].bits.present == 1){
        new_table = (Page_entry*)(pte_dir + frame_size * page_directory_index);
        pde[page_directory_index].bits.user_supervisor = is_user ? 1 : 0;

    }
    
    else{
        unsigned int table_physical_addr = alloc_frame();

        if (table_physical_addr == null)
        {
            set_bitmap( physical_addr >> 12  , 0);
            return 0;
        }

        pde[page_directory_index].bits.frame = table_physical_addr >> 12;
        pde[page_directory_index].bits.present = 1;
        pde[page_directory_index].bits.read_write = 1;
        pde[page_directory_index].bits.user_supervisor = is_user ? 1 : 0;

        new_table = (Page_entry *)(pte_dir + frame_size * page_directory_index);
        __asm__ __volatile__("invlpg (%0)" : : "r"(new_table) : "memory");
        // איפוס ערכי זבל
        for(int i = 0 ; i < 1024 ; i++){
            new_table[i].value = 0;
        }
        
    }
    
    new_table[page_table_index].bits.present = 1;
    new_table[page_table_index].bits.read_write = 1;
    new_table[page_table_index].bits.user_supervisor = is_user ? 1 : 0;
    new_table[page_table_index].bits.frame = (physical_addr >> 12);

    // TLB Cache מודיעה למעבד שהשתנה מיפוי ספציפי בזיכרון, כדי שיעדכן את ה
    __asm__ __volatile__("invlpg (%0)" : : "r"(virtual_addr) : "memory");
    return 1;
}

void *kmalloc(unsigned int size)
{
    if (size == 0)
        return null;

    size = ALIGN8(size);
    unsigned int header_size = ALIGN8(sizeof(Heap_header));
    unsigned int total_size = size + header_size;
    Heap_header *current = kernel_heap_start_block;
    Heap_header *last = null;

    // חיפוש בלוק פנוי
    while (current != null)
    {
        if (current->is_free)
        {
            // בלוק שמתאים בדיוק לגודל
            if (current->size >= size && current->size < total_size + header_size)
            {
                current->is_free = 0;
                return (void *)((unsigned int)current + header_size);
            }
            // בלוק גדול מדי ויש לפצל אותו
            else if (current->size >= total_size + header_size)
            {
                Heap_header *next_block = (Heap_header *)((unsigned int)current + total_size);
                next_block->is_free = 1;
                next_block->size = current->size - total_size;
                next_block->next = current->next;
                next_block->prev = current;

                current->next = next_block;
                current->size = size;
                current->is_free = 0;

                if (next_block->next != null)
                {
                    next_block->next->prev = next_block;
                }

                return (void *)((unsigned int)current + header_size);
            }
        }
        last = current;
        current = current->next;
    }
    return null;
}


void *_malloc(unsigned int size, Thread_list* th)
{
    if(th && th->thread && th->thread->process == kernel_processe){
        return kmalloc(size);
    }

    unsigned int header_size = ALIGN8(sizeof(Heap_header));
    if (size == 0 || size > (0xFFFFFFFF - header_size - 8))
    {
        return null;
    }
    size = ALIGN8(size);

    unsigned int total_size = size + header_size;

    if(th == null || th->thread == null ||
    th->thread->process == null)
    {
        return null;
    }
    Heap_header *current = th->thread->process->heap;

    Heap_header *last = null;
    // חיפוש בלוק פנוי
    while (current != null)
    {
        if (current->is_free)
        {
            //בלוק שמתאים בדיוק לגודל
            if (current->size >= size && current->size < total_size + header_size)
            {
                current->is_free = 0;
                return (void *)((unsigned int)current + header_size);
            }
            // בלוק גדול מדי ויש לפצל אותו
            else if (current->size >= total_size + header_size)
            {
                Heap_header *next_block = (Heap_header *)((unsigned int)current + total_size);
                next_block->is_free = 1;
                next_block->size = current->size - total_size;
                next_block->next = current->next;
                next_block->prev = current;

                current->next = next_block;
                current->size = size;
                current->is_free = 0;

                if (next_block->next != null)
                {
                    next_block->next->prev = next_block;
                }

                return (void *)((unsigned int)current + header_size);
            }
        }
        last = current;
        current = current->next;
    }

    // אם לא נמצא בלוק פנוי וצריך להקצות דפים חדשים 
    unsigned int pages_needed = (total_size + frame_size - 1) / frame_size;
    unsigned int allocated_space = pages_needed * frame_size;

    Process* p = th->thread->process;

    unsigned int addr = p->heap_start;
    p->heap_start += allocated_space;
    VM_area *vm = null;

    if(p->heap != null){
        vm = find_vma(p->vma, addr - 1);
    }
    else{
        vm = find_vma(p->vma, addr);
    }

    if (vm != null)
    {
        vm->end = p->heap_start;
    }
    else
    {
        add_vma(&(p->vma), addr, p->heap_start, 0x7, "user_heap");
    }
    
    Heap_header *new_node = (Heap_header *)addr;

    new_node->is_free = 0;
    new_node->next = null;
    new_node->size = allocated_space - header_size;

    if (p->heap == null)
    {
        
        p->heap = new_node;
        new_node->prev = null;
    }
    else if (last != null)
    {
        
        last->next = new_node;
        new_node->prev = last;
    }

    if (allocated_space >= total_size + header_size + max_bytes_gap)
    {
        new_node->size = size;

        Heap_header *_new_node = (Heap_header *)((unsigned int)new_node + total_size);
        _new_node->is_free = 1;
        _new_node->next = null;
        _new_node->prev = new_node;

        _new_node->size = allocated_space - total_size - header_size;

        new_node->next = _new_node;
    }

    return (void *)((unsigned int)new_node + header_size);
}

void* malloc(unsigned int size){
   return _malloc(size , th_list);
}


void memory_init()
{
    physical_memory_init();
    paging_init();

    //סוף הקרנל
    unsigned int addr = (((unsigned int)&kernel_end) & 0xFFFFF000) + kb4;
    unsigned int header_sz = ALIGN8(sizeof(Heap_header));

    kernel_heap_start_block = (Heap_header *)addr;
    kernel_heap_start_block->is_free = 1;
    kernel_heap_start_block->prev = null;
    kernel_heap_start_block->next = null;
    kernel_heap_start_block->size = kernel_limit - addr - header_sz;

    add_vma(&(vma_list) ,0x0, addr, 0xF, "kernel");
    add_vma(&(vma_list), addr, kernel_limit, 0xF, "kernel_heap");
    add_vma(&(vma_list), user_heap_start_addr, user_heap_start_addr + 1, 0xf, "user_heap");
}


void __free(void *ptr , Thread_list* th)
{
    if (ptr == null)
        return;

    int header_sz = ALIGN8(sizeof(Heap_header));

    Heap_header *current = (Heap_header *)((unsigned int)ptr - header_sz);

    current->is_free = 1;

    // איחוד קדימה (עם הבלוק הבא ברשימה פנוי)
    if (current->next != null && current->next->is_free)
    {
        current->size += current->next->size + header_sz;
        current->next = current->next->next;

        if (current->next != null)
        {
            current->next->prev = current;
        }
    }

    // איחוד אחורה (עם הבלוק הקודם ברשימה פנוי)
    if (current->prev != null && current->prev->is_free)
    {
        Heap_header *prev_node = current->prev;
        prev_node->size += current->size + header_sz;
        prev_node->next = current->next;

        if (current->next != null)
        {
            current->next->prev = prev_node;
        }

        current = prev_node;
    }

    //מסיר את המיפוי של הזיכרון אם זה האיזור האחרון שיש לו מיפוי הזיכרון של המשתמש
    if( (unsigned int)ptr >= kernel_limit &&current->next == null && current->size > frame_size + max_bytes_gap
        && th && th->thread && th->thread->process){
            Process* p = th->thread->process;
            unsigned int block_start = (unsigned int)current;
            unsigned int block_end = block_start + current->size + header_sz;
            // מעגל את ההתחלה כלפי מטה ואת סוף כלפי מעלה 
            unsigned int start_addr = block_start & ~(frame_size - 1);
            unsigned int end_addr = (block_end + frame_size - 1) & ~(frame_size - 1);
            unsigned int num_pages = (end_addr - start_addr) / frame_size;

            //ניתוק העמוד לפני שמסירים את המיפוי של הזיכרון
            if(current->prev != null){
                current->prev->next = null;
            }
            else{
                p->heap = null;
            }

            for(unsigned int i = 0 ; i < num_pages ; i++){
                unmap(start_addr + i * frame_size);
            }
            if(p->heap_start == end_addr){
                p->heap_start = start_addr;
            }
            if(p->vma != null){
                VM_area* vm = find_vma(p->vma, start_addr);
                if(vm != null && vm->end == end_addr){
                    vm->end = start_addr;
                    if(vm->start == vm->end){
                        vm->end += 1;// מונע מצב שבו אין יותר מיפוי של הזיכרון לאיזור זה           
                    }
                }
            }
    }
}

void _free(void* ptr){
    __free(ptr, th_list);
}



void unmap(unsigned int virtual_addr){

    virtual_addr &= 0xFFFFF000;

    unsigned int pde_i = virtual_addr >> 22;
    unsigned int pte_i = (virtual_addr >> 12) & 0x3FF;
    Page_entry *table = (Page_entry *)(pte_dir + frame_size * pde_i);
    Page_entry* pde = (Page_entry*)pde_addr;
    if (pde[pde_i].bits.present == 0)
    {
        return;
    }

    if(table[pte_i].bits.present != 0){
        set_bitmap(table[pte_i].bits.frame, 0);
        table[pte_i].value = 0;

        // TLB Cache מודיעה למעבד שהשתנה מיפוי ספציפי בזיכרון, כדי שיעדכן את ה
        __asm__ __volatile__("invlpg (%0)" : : "r"(virtual_addr) : "memory");
    }
}

void *realloc(void *ptr, unsigned int size)
{
    if (ptr == null)
    {
        return null;
    }

    if (size == 0)
    {
        free(ptr);
        return null;
    }
    Heap_header *block = (Heap_header *)((unsigned int)ptr - ALIGN8(sizeof(Heap_header)));

    void* addr = null;

    if((unsigned int)ptr < kernel_limit){
        addr = kmalloc(size);
    }
    else{
        addr = malloc(size);
    }

    if(addr == null) return null;

    memcopy(addr , ptr , min(size , block->size));
    free(ptr);
    return addr;
}


void memset(void* ptr , int value, unsigned int size)
{
    unsigned char* p = (unsigned char*)ptr;
    for(unsigned int i = 0 ; i < size ; i++){
        p[i] = (unsigned char)value;
    }
}


void memcopy(void *target, void *source, unsigned int size)
{
    char *ptr = (char *)target;
    char *s = (char *)source;

    for (int i = 0; i < size; i++)
    {
        ptr[i] = s[i];
    }
}

unsigned int calc_heap_size(Heap_header* heap){
    unsigned int size = 0;

    while (heap != null)
    {
        if(!heap->is_free){
            size += heap->size;
        }
        heap = heap->next;
    }
    return size;
    
}


