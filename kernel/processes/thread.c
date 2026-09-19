#include "thread.h"
#include "memory.h"
#include "stdio.h"
#include "process.h"
#include "tss.h"
#include "idt.h"
#include "timer.h"
#include "VM_area.h"
#include "Queue.h"

void remove_current_thread();
void set_current_as_dead();
unsigned int next_id = 0;

Thread_list* th_list = null;


unsigned int* put_args_in_stack(void* top_stack, va_list args ){

    int count = 0;
    void* temp_args[10];
    void* temp;
    
    while ( count < 10)
    {
        temp = va_arg(args, void*);
        if((unsigned int)temp == __UINT32_MAX__)break;
        temp_args[count++] = temp;
    }

    if(count == 0) return (unsigned int *)top_stack;

    unsigned int* stack = (unsigned int*)top_stack;

    for(int i = count -1 ; i >= 0; i--){
        stack--;
        *stack = (unsigned int)temp_args[i];
    }
    
    return (unsigned int *)stack;
}


Thread *_create_thread(thread_entry_t entry_point , ...)
{
    va_list args;
    va_start(args , entry_point);

    Process *p = null;
    if (th_list != null && th_list->thread != null)
    {
        p = th_list->thread->process;
    }

    Thread* t = create_thread_for_processe(entry_point, p, args);
    va_end(args);
    return t;
}



Thread *create_thread_for_processe(thread_entry_t entry_point, Process* p , va_list args)
{
    if(entry_point == null) return null;

    Thread *thread = kmalloc(sizeof(Thread));
    if(thread == null) return null;
    thread->lock_deep = -1;
    thread->stack = kmalloc(STACK_SIZE);

    if(thread->stack == null){
        free(thread);
        return null;
    }

    // כדי לחשב את המיקום בביטים בודדים char שימוש ב
    char *top_stack = (char *)thread->stack + STACK_SIZE;
    thread->top_esp = (unsigned int)top_stack;


    unsigned int *addr = put_args_in_stack((void*)top_stack, args);
    addr--;
    *addr = (unsigned int)set_current_as_dead;

    Stack_frame *frame = (Stack_frame *)((char *)addr - sizeof(Stack_frame));

    frame->eip = (unsigned int)entry_point; // נקודת החזרה
    frame->cs = 0x08;
    frame->eflags = 0x200; // מאפשר פסיקות מערכת

    // איפוס האוגרים הכלליים המזויפים כדי למנוע ערכי זבל
    frame->edi = 0;
    frame->esi = 0;
    frame->ebp = 0;
    frame->ebx = 0;
    frame->esp_dummy = 0;
    frame->edx = 0;
    frame->ecx = 0;
    frame->eax = 0;

    thread->esp = frame;
    thread->state = BLOCKED;

    thread->id = next_id;
    next_id++;

    thread->process = null;

    Thread_list *new_node = kmalloc(sizeof(Thread_list));
    if(new_node == null){
        free(thread->stack);
        free(thread);
        return null;
    }
    new_node->thread = thread;

    if (th_list == null)
    {
        th_list = new_node;
        new_node->next = new_node;
        new_node->prev = new_node;
        thread->process = null;
    }

    else
    {
        asm volatile("cli");
    
        new_node->next = th_list->next;

        new_node->prev = th_list;

        th_list->next->prev = new_node;

        th_list->next = new_node;

        thread->process = p;
        if (thread->process != null)
        {
            k_insert_q(thread->process->threads, thread);
        }
        asm volatile("sti");
    }
    return thread;
}

void* switch_thread(void* esp){
    if (th_list == null)
    {
        return esp;
    }
    if(th_list->thread != null){

        th_list->thread->esp = esp;
    }

    if(th_list->thread->state == RUNNING){
        th_list->thread->state = READY;
    }

    th_list = th_list->next;

    while (th_list != null)
    {
        if(th_list->thread == null || th_list->thread->state == ZOMBIE )
        {
            remove_current_thread();
            continue;
        }

        if(th_list->thread->state == BLOCKED)
        {

            th_list = th_list->next;
            continue;
        }

        if(th_list->thread->state == READY){

            break;
        }
        
    }

    th_list->thread->state = RUNNING;
    
    tss.esp0 = (unsigned int)th_list->thread->top_esp;
    
    
    return th_list->thread->esp;
}


void init_threading()
{
    unsigned int kernel_esp;

    __asm__ volatile("mov %%esp, %0" : "=r"(kernel_esp));

    tss_init(KERNEL_DATA_SEGMENT, kernel_esp);
    // יצירת התהליך הראשי
    Thread *main_thread = kmalloc(sizeof(Thread));
    main_thread->state = RUNNING;
    main_thread->id = next_id++;
    main_thread->stack = null;
    main_thread->top_esp = kernel_esp;

    Thread_list *main_node = kmalloc(sizeof(Thread_list));
    main_node->thread = main_thread;
    main_node->next = main_node; // הפיכה לרשימה מעגלית
    main_node->prev = main_node;

    // יצירת תהליך הקרנל
    Process* p = kmalloc(sizeof(Process));

    if(p == null){
        th_list = main_node;
        return;
    }

    p->heap = kernel_heap_start_block;
    p->heap_start = null;
    p->id = -1;
    p->parent = null;
    p->pde_physical_address = pde_physical_address;
    p->threads = create_queue();

    k_insert_q(p->threads, main_thread);
    p->vma = vma_list;
    kernel_processe = p;
    main_thread->process = p;
    
    processe_q = _create_queue(1);
    k_insert_q(processe_q , p);
    
    th_list = main_node;
    start_thread(create_thread(kill_processe_idle));
}

void remove_current_thread()
{
    if(th_list == null)return;

    Thread_list *current = th_list;

    if (current->next == current)
    {
        th_list = null;
    }

    else
    {
        current->prev->next = current->next;
        current->next->prev = current->prev;
        th_list = current->next;
    }
    free_thread(current->thread);
    free(current);
}


void set_current_as_dead(){

    if(th_list != null && th_list->thread != null){
        th_list->thread->state = ZOMBIE;
    }
    yield;

    // ליתר ביטחון במקרה שההסרה תיכשל והקוד יחזור להריץ את הקוד שפה
    while (1)
    {
        __asm__ volatile("hlt");
    }
}
void set_thread_state(Thread* th, Thread_state state){
    if(th == null) return;

    th->state = state;
}

void join_thread(Thread* th){

    while (th != null && th->state != ZOMBIE)
    {
        yield;
    }
}

void start_thread(Thread* th){
    if (th == null)
    {
        return;
    }
    th->lock_deep = 0;

    if(th->state == BLOCKED){
        th->state = READY;
    }
}


Lock_t* create_lock()
{

    Lock_t* lock = kmalloc(sizeof(Lock_t));
    if(lock == null){
        return null;
    }

    lock->lock_count = 0;
    lock->waiting_threads = _create_queue(1);
    lock->owner_id = -1;
    lock->recursive_lock_count = 0;

    return lock;
}

/*
    מימוש של מנעול עם תמיכה בנעילה רקורסיבית(אותו התהליך יכול לנעול את המנעול מספר פעמים מבלי להיתקע)
    המנעול שומר על מספר הפעמים שהתהליך הנעול את המנעול נעול בו, ומאפשר לו לשחרר את המנעול רק כאשר הוא שחרר את כל הנעילות שלו כדי
    שתהליך לא ישתמש במנעול פעמיים ויתקע את עצמו
*/

void Lock(Lock_t* lock)
{
    __asm__ volatile("cli");

    if(lock == null){
        __asm__ volatile("sti");
        return;
    }

    if(lock->lock_count == 0){
        
        lock->lock_count = 1;
        lock->owner_id = th_list->thread->id;
        lock->recursive_lock_count = 1;

        __asm__ volatile("sti");
        return;
    }

    if(lock->owner_id == th_list->thread->id){
        lock->recursive_lock_count++;
        __asm__ volatile("sti");
        return;
    }

    Thread* current_thread = th_list->thread;

    insert_thread_to_lock_queue(lock, current_thread);

    __asm__ volatile("sti");

    yield;
}


void Unlock(Lock_t* lock)
{
    __asm__ volatile("cli");

    if(lock == null){
        __asm__ volatile("sti");
        return;
    }

    if(lock->lock_count == 0 || lock->owner_id != th_list->thread->id){
        __asm__ volatile("sti");
        return;
    }

    lock->recursive_lock_count--;

    if(lock->recursive_lock_count > 0){
        __asm__ volatile("sti");
        return;
    }

    if(lock->waiting_threads != null && lock->waiting_threads->Len == 0){

        lock->lock_count = 0;
        lock->owner_id = -1;
    }
    else{
        remove_thread_from_lock_queue(lock);
    }


    __asm__ volatile("sti");
}

void insert_thread_to_lock_queue(Lock_t* lock, Thread* thread)
{
    if(lock == null || thread == null){
        return;
    }

    thread->lock_deep++;
    thread->state = BLOCKED;
    
    k_insert_q(lock->waiting_threads, thread);
}


void remove_thread_from_lock_queue(Lock_t* lock)
{
    if(lock == null || lock->waiting_threads == null){
        return;
    }
    Thread* th = pop_q(lock->waiting_threads);

    if(th == null){
        return;
    }

    lock->owner_id = th->id;
    lock->recursive_lock_count = 1;    

    th->lock_deep--;

    if(th->lock_deep <= 0){
        th->lock_deep = 0;
        th->state = READY;
    }
}

void free_thread(Thread* th){
    if(th == null) return;

    if(th_list != null){
        Thread_list* current = th_list;
        do{
            if(current->thread == th){
                if(current->next == current){
                    th_list = null;
                }
                else{

                    asm volatile("cli");
                    current->prev->next = current->next;
                    current->next->prev = current->prev;
                    th_list = current->next;
                    asm volatile("sti");
                }
                free(current);
                break;
            }
            current = current->next;
        }while(current != th_list && th_list != null);
    }

    if(th->process != null && th->process->threads != null){
        remove_by_val_q(th->process->threads, th);
        if(th->process->threads->Len == 0){
            th->process->state = DEAD;
        }
    }
    
    free(th->stack);
    free(th);
}

/*
================== לא עובד ==================

Thread *create_user_thread(void (*entry_point)())
{
    Thread *thread = kmalloc(sizeof(Thread));
    if (thread == null)
        return null;

    thread->lock_deep = -1;
    thread->stack = malloc(STACK_SIZE);
    thread->k_stack = kmalloc(STACK_SIZE);
    if (thread->stack == null || thread->k_stack == null)
    {
        free(thread->stack);
        free(thread->k_stack);
        free(thread);
        return null;
    }

    char *top_user_stack = (char *)thread->stack + STACK_SIZE;

    char *top_kernel_stack = (char *)thread->k_stack + STACK_SIZE;
    thread->top_esp = (unsigned int)top_kernel_stack;

    User_stack_frame *frame = (User_stack_frame *)(top_kernel_stack - sizeof(User_stack_frame));
    memset(frame, 0, sizeof(User_stack_frame));

    frame->cs = USER_CODE_SEGMENT;
    frame->ss = USER_DATA_SEGMENT;
    frame->eflags = 0x201;

    frame->eip = (unsigned int)entry_point;
    frame->user_esp = (unsigned int)top_user_stack;

    thread->esp = frame;
    thread->state = BLOCKED;
    thread->id = next_id++;
    thread->process = null;


    Thread_list* new_node = kmalloc(sizeof(Thread_list));
    new_node->thread = thread;
    if (th_list == null)
    {
        th_list = new_node;
        new_node->next = new_node;
        new_node->prev = new_node;
        thread->process = null;
    }

    else
    {
        new_node->next = th_list->next;

        new_node->prev = th_list;

        th_list->next->prev = new_node;

        th_list->next = new_node;

        thread->process = th_list->thread->process;
        if (thread->process != null)
        {
            insert_q(thread->process->threads, thread);
        }
    }

    return thread;
}
*/
