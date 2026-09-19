#include "process.h"
#include "thread.h"
#include "memory.h"
#include "stdio.h"
#include "physical_memory.h"
#include "VM_area.h"

extern int kernel_end; 

unsigned int next_process_id = 0; // ה 1- תמיד שמור לקרנל
Process* kernel_processe = null;
Queue_s* processe_q;

unsigned int get_cr3()// את טבלת הדפים EAX שם ב
{

    if (th_list == null || th_list->thread == null || th_list->thread->process == null)
    {
        return 0;
    }
    
    return th_list->thread->process->pde_physical_address;
}

#define mapping_index 1022

unsigned int create_page_directroy(){

    __asm__ volatile("cli");
    Page_entry *pde = (Page_entry *)pde_addr;

    unsigned int temp = pde[mapping_index].value;//שומר את הערך הנוכחי כדי לא לפגוע במידע שיש שם

    unsigned int phys_addr = alloc_frame();

    if (phys_addr == 0 || (phys_addr & 0xFFF) != 0)
    {
        return 0;
    }
    pde[mapping_index].bits.read_write = 1;
    pde[mapping_index].bits.present = 1;
    pde[mapping_index].bits.frame = (phys_addr >> 12);

    Page_entry *new_directory = (Page_entry *)(pte_dir + mapping_index * frame_size);
    __asm__ __volatile__("invlpg (%0)" : : "r"(new_directory) : "memory");//ריענון זיכרון של הטבלה

    memset(new_directory , 0 , sizeof(Page_entry) * kb1);

    unsigned int kernel_pages = kernel_limit / mb4;

    for(int i = 0; i< kernel_pages; i++){
        new_directory[i].value = page_directory[i].value;//מיפוי הקרנל
    }
    //recursive mapping
    new_directory[1023].bits.present = 1;
    new_directory[1023].bits.read_write = 1;
    new_directory[1023].bits.frame = phys_addr>>12;

    pde[mapping_index].value = temp;//שיחזור הטבלה

    __asm__ __volatile__("invlpg (%0)" : : "r"(new_directory) : "memory");

    __asm__ volatile("sti");

    return phys_addr;//החזרת מיקום הפיזי של הטבלה
}


Thread* _create_processe(void(* entery_point)(), ...){
    
    if(entery_point == null) return null;

    Process* p = kmalloc(sizeof(Process));
    if(p == null) return null;

    p->state = ALIVE;
    p->heap_start = kernel_limit;
    p->heap = null;

    if(th_list != null && th_list->thread != null){
        p->parent = th_list->thread->process;
    }
    else{
        p->parent = null;
    }
    p->threads = _create_queue(1);

    if(p->threads == null){

        free(p);
        return null;
    }

    va_list args;
    va_start(args , entery_point);
    Thread* th = create_thread_for_processe(entery_point, p, args);
    va_end(args);

    if(th == null){

        free_q(p->threads);
        free(p);
        return null;
    }

    p->id = next_process_id;
    next_process_id += 1;

    p->pde_physical_address = create_page_directroy();

    if (p->pde_physical_address == 0)
    {
        free_thread(th);
        return null;
    }


    p->vma = null;
    unsigned int addr = (((unsigned int)&kernel_end) & 0xFFFFF000) + kb4;
    add_vma(&(p->vma), addr, kernel_limit, 0xF, "kernel_heap");
    add_vma(&(p->vma), kernel_limit, kernel_limit + 1, 0xF, "user_heap");

    k_insert_q(processe_q , p);

    return th;
}



void free_processe(Process* p){
    
    if(p == null) return;

    __asm__ volatile("cli");

    Queue_s* q = p->threads;
    if(q != null){

        while (q->Len != 0)
        {
            set_thread_state(pop_q(q), ZOMBIE);
        }
        free_q(q);
    }

    unsigned int temp_pde = page_directory[mapping_index].value;
    unsigned int temp_pte = page_directory[mapping_index - 1].value;
    page_directory[mapping_index].bits.present = 1;
    page_directory[mapping_index].bits.read_write = 1;
    page_directory[mapping_index].bits.frame = (p->pde_physical_address >> 12);


    Page_entry* pde = (Page_entry *)(pte_dir + mapping_index * frame_size);
    Page_entry *pte;
    __asm__ __volatile__("invlpg (%0)" : : "r"(pde) : "memory"); // ריענון זיכרון של הטבלה

    int start_pde_i = (kernel_limit + mb4 -1) / mb4 ;// מוודא שההתחלה מתבצעת לאחר הקרנל

    for (int pde_i = start_pde_i; pde_i < kb1 - 1; pde_i++)
    {
        if(pde[pde_i].bits.present == 1){

            page_directory[mapping_index - 1].bits.present = 1;
            page_directory[mapping_index - 1].bits.read_write = 1;
            page_directory[mapping_index - 1].bits.frame = pde[pde_i].bits.frame;

            pte = (Page_entry*)(pte_dir + (mapping_index-1)*frame_size);
            __asm__ __volatile__("invlpg (%0)" : : "r"(pte) : "memory"); // ריענון זיכרון של הטבלה
            
            for(int i = 0 ; i < kb1; i++){
                if(pte[i].bits.present == 1){
                    set_bitmap(pte[i].bits.frame, 0);// מחיקת הזיכרון של כל עמוד בטבלה
                    pte[i].value = 0;
                }
            }
            set_bitmap(pde[pde_i].bits.frame , 0);// מחיקת הזיכרון של הטבלה עצמה
            pde[pde_i].value = 0;
            page_directory[mapping_index - 1].value = 0;
            __asm__ __volatile__("invlpg (%0)" : : "r"(pte) : "memory");
        }
    }

    set_bitmap(p->pde_physical_address >> 12 , 0);// מחיקת הזיכרון הטבלה של התהליך

    // שיחזור הטבלאות המקוריות
    page_directory[mapping_index - 1].value = temp_pte;
    page_directory[mapping_index].value = temp_pde;

    __asm__ __volatile__("invlpg (%0)" : : "r"(pde) : "memory");
    __asm__ __volatile__("invlpg (%0)" : : "r"(pte) : "memory");

    VM_area *vm;
    if(p->vma != null){
        while(p->vma != null){
            vm = p->vma;
            p->vma = p->vma->next;
            free(vm);
        }
    }
    remove_by_val_q(processe_q, p);
    free(p);

    __asm__ volatile("sti");
}

void kill_processe_idle(){
    
    List_s* current;
    List_s* next;
    Process* p;

    while(1){
        if(!processe_q) continue;

        current = processe_q->end_node;
        while(current != null){
            next = current->next;
            p = (Process*)current->val;
            if(p != null && p->state == DEAD && p != kernel_processe){

                free_processe(p);
            }

            current = next;
            
        }
    }
}