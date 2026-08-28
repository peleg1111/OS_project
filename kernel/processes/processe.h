#ifndef PROCESSE_H
#define PROCESSE_H

#include "Queue.h"
#include "memory.h"
#include "thread.h"

struct VM_area;
struct Heap_header;
struct Thread;

typedef struct Processe {

    unsigned int id;
    unsigned int pde_physical_address;//כתובת פיזית לעידכון  מיפוי הדפים בהחלפת תהליך
    Queue_s* threads;

    struct VM_area* vma;

    struct Processe* parent;
    Heap_header* heap;
    unsigned int heap_start;

} Processe;

extern Processe* kernel_processe;
extern Queue_s* processe_q;

unsigned int get_cr3();
struct Thread* create_processe(void(* entery_point)());
void free_processe(Processe* p);

#endif