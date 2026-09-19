#ifndef THREAD_H
#define THREAD_H

#include "Queue.h"
#include "process.h"

struct Process;

#define STACK_SIZE 4096

typedef enum
{
    READY = 0,
    RUNNING = 1,
    BLOCKED = 2,
    ZOMBIE = 3

} Thread_state;

typedef struct
{
    // נדחפים באופן ידני
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp_dummy;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    unsigned int eip;    // כתובת החזרה
    unsigned int cs;     // Code Segment
    unsigned int eflags; // אוגר המצבים

} __attribute__((packed)) Stack_frame;




typedef struct Thread{

    unsigned int id;
    unsigned int top_esp;
    int lock_deep; 
    Thread_state state;
    void* esp;
    struct Process* process;
    void* stack;
    void* k_stack;

}Thread;

typedef struct Thread_list
{
    Thread *thread;
    struct Thread_list *next;
    struct Thread_list *prev;

} Thread_list;

extern Thread_list *th_list;

typedef struct{
    
    int lock_count;
    unsigned int owner_id;
    unsigned int recursive_lock_count;
    Queue_s *waiting_threads;

}Lock_t;


void *switch_thread(void *esp);

typedef void (*thread_entry_t)();// מצביעה לפונקציה שמקבלת כמות משתנה של פרמטרים ולא מחזירה כלום

Thread *_create_thread(thread_entry_t entry_point, ...);

// ## -->> ריק __VA_ARGS__ לא מעביר פרמטר עם
#define create_thread(func, ...) _create_thread((thread_entry_t)(func), ##__VA_ARGS__, __UINT32_MAX__)

void init_threading();

void join_thread(Thread *th);

void start_thread(Thread *th);

void Unlock(Lock_t *lock);

void Lock(Lock_t *lock);

Lock_t *create_lock();

void insert_thread_to_lock_queue(Lock_t *lock, Thread *thread);

void remove_thread_from_lock_queue(Lock_t *lock);

void set_current_as_dead();

void free_thread(Thread* th);

typedef __builtin_va_list va_list;
unsigned int* put_args_in_stack(void* top_stack, va_list args );

Thread *create_thread_for_processe(thread_entry_t entry_point, struct Process *p, va_list args);
void set_thread_state(Thread *th, Thread_state state);


#endif