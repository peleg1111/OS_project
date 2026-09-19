#include "process_test.h"
#include "stdio.h"
#include "memory.h"
#include "thread.h"
#include "process.h"
#include "timer.h"

void _test_process_free_helper()
{

    for (int i = 0; i < 40; i++)
    {
        time.sleep(0.01);
        malloc(1030);
        if(th_list && th_list->thread && th_list->thread->process){
            printf("\nheap size: %d", calc_heap_size(th_list->thread->process->heap));
        }
    }
}

void test_process_free_helper()
{
    time.sleep(1.5);
    Thread *t2 = create_thread(_test_process_free_helper);
    Thread *t3 = create_thread(_test_process_free_helper);
    start_thread(t2);
    start_thread(t3);

    join_thread(t2);
    join_thread(t3);

    printf("\nt3 end");
    header_msg();
}

void test_process_free()
{
    unsigned int heap_start_size = calc_heap_size(kernel_heap_start_block);

    int num = 10;
    Thread **t = malloc(sizeof(Thread) * num);
    int size = get_free_frames_count();
    for (int i = 0; i < num; i++)
    {
        t[i] = create_processe(test_process_free_helper);

        start_thread(t[i]);
    }

    for (int i = 0; i < num; i++)
    {
        join_thread(t[i]);
    }

    for (int i = 0; i < 100; i++)//ממתין לשיחרור הזיכרון
    {
        yield;
    }
    free(t);

    printf("\n\t free frames before test = %d  free frames after test = %d", size, get_free_frames_count());
    printf("\n\t heap size before test = %d  heap size after test = %d", heap_start_size, calc_heap_size(kernel_heap_start_block));
}


void test_process_running_process(){
    Thread* t = create_processe(test_process_free);
    start_thread(t);
    join_thread(t);
}