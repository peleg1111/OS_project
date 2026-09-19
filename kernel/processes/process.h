#ifndef PROCESSE_H
#define PROCESSE_H

#include "Queue.h"
#include "memory.h"
#include "thread.h"

struct VM_area;
struct Heap_header;
struct Thread;

typedef enum {
    ALIVE,
    DEAD
} Processe_state;


typedef struct Process {

    long id;
    unsigned int pde_physical_address;//כתובת פיזית לעידכון  מיפוי הדפים בהחלפת תהליך
    Queue_s* threads;

    struct VM_area* vma;

    struct Process* parent;
    Heap_header* heap;
    unsigned int heap_start;
    Processe_state state;

} Process;

extern Process* kernel_processe;
extern Queue_s* processe_q;

unsigned int get_cr3();

typedef void (*thread_entry_t)(); // מצביעה לפונקציה שמקבלת כמות משתנה של פרמטרים ולא מחזירה כלום

struct Thread* _create_processe(thread_entry_t entery_point, ...);

#define create_processe(entery_point, ...) _create_processe((thread_entry_t)entery_point, ##__VA_ARGS__, __UINT32_MAX__)

void kill_processe_idle();
void kill_processe(Process* p);
void free_processe(Process* p);

#endif