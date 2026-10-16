#include "process_test.h"
#include "stdio.h"
#include "memory.h"
#include "thread.h"
#include "process.h"
#include "timer.h"



#define WAIT_LOOPS 250
#define LEAK_TOLERANCE 5 // עבוד גדילה של הזיכרון ממקור חיצוני
#define LOCK_WORKERS 4
#define LOCK_ITER 10
#define TEST_ADDR 0xE3000000

static int passed = 0;
static int failed = 0;
static int total_tests = 28;

static volatile int g_flag = 0;    // שרץ thread הערך מסמל את ה
static volatile int g_stage = 0;   // threads שלב בסנכרון בין
static volatile int g_result = 0;  // thread חישוב התוצאה של ה
static volatile int g_counter = 0; // מונה שמוגן על ידי מנעול
static volatile int g_done = 0;    // שסיימו threads
static volatile int g_runner_done = 0;
static Lock_t *g_lock = null;

// ==========    פונקציות עזר    ==========
#define check(condition, name)           \
    do                                   \
    {                                    \
        if (condition)                   \
        {                                \
            passed++;                    \
            printf("[PASS] %s\n", name); \
        }                                \
        else                             \
        {                                \
            failed++;                    \
            printf("[FAIL] %s\n", name); \
        }                                \
    } while (0)

// מחכה עד שהמשתנה מגיע לערך המבוקש. מחזיר 1 אם הצליח, 0 אם נגמר הזמן
static int wait_for(volatile int *var, int value)
{
    for (int i = 0; i < WAIT_LOOPS; i++)
    {
        if (*var >= value) return 1;
        yield;
    }
    return 0;
}

// thread מחכה לניקוי ה 
static int wait_thread_gone(unsigned int id)
{
    for (int i = 0; i < WAIT_LOOPS; i++)
    {
        asm("cli");
        if (get_thread_by_id(id) == null) return 1;
        asm("sti");
        yield;
    }
    return 0;
}

// process מחכה לניקוי ה
static int wait_process_gone(long id)
{
    for (int i = 0; i < WAIT_LOOPS; i++)
    {
        if (get_process_by_id(id) == null) return 1;
        yield;
    }
    return 0;
}

//בודק את תקינות הרשימה עצמה
static int count_threads()
{
    int count = 0;
    asm volatile("cli");
    Thread_list *node = th_list;
    do
    {
        if (node->next->prev != node || node->prev->next != node)
        {
            count = -1;
            break;
        }
        count++;
        node = node->next;
    } while (node != th_list);
    asm volatile("sti");
    return count;
}

static void free_lock(Lock_t *lock)
{
    free_q(lock->waiting_threads);
    free(lock);
}

// ==========    thread בדיקות    ==========

static void worker_set_flag()
{
    g_flag = 1;
}

static void worker_spin()
{
    g_stage = 1;
    while (1) yield;
}

// מגדיל את המונה, ומחליף הקשר באמצע כדי לחשוף מצב שבו אין נעילה
static void worker_locked_increment()
{
    for (int i = 0; i < LOCK_ITER; i++)
    {
        Lock(g_lock);
        int value = g_counter;
        yield;
        g_counter = value + 1;
        Unlock(g_lock);
    }
    Lock(g_lock);
    g_done++;
    Unlock(g_lock);
}

static void worker_lock_once()
{
    g_stage = 1;
    Lock(g_lock);
    g_stage = 2;
    Unlock(g_lock);
}

// בודק רישום של תהליכון בתוך תהליך
static void worker_spawn_child()
{
    Process *p = th_list->thread->process;
    Thread *child = create_thread(worker_set_flag);
    if (child == null) return;

    g_result = (child->process == p && p->threads->Len == 2);
    start_thread(child);
    g_stage = 1;
}

//בדיקת כתיבה לאותה כתובת ווירטואלית בשני תהליכים שונים
static void worker_isolation_a()
{
    if (!map_page(TEST_ADDR, 0)) return;
    volatile unsigned int *p = (volatile unsigned int *)TEST_ADDR;
    *p = 0xAAAA;
    g_stage = 1;
    wait_for(&g_stage, 2);
    g_result = (*p == 0xAAAA);
    unmap(TEST_ADDR);
    g_stage = 3;
}

static void worker_isolation_b()
{
    wait_for(&g_stage, 1);
    if (!map_page(TEST_ADDR, 0)) return;
    volatile unsigned int *p = (volatile unsigned int *)TEST_ADDR;
    *p = 0xBBBB;
    unmap(TEST_ADDR);
    g_stage = 2;
}

// ==========    Threads    ==========

void test_thread_create()
{
    int before = count_threads();

    check(create_thread(null) == null, "create_thread rejects a null entry point");

    Thread *t = create_thread(worker_set_flag);
    check(t != null, "create_thread returns a thread");
    if (t == null) return;
    unsigned int id = t->id;

    check(t->state == BLOCKED, "New thread starts BLOCKED");
    check(t->process == th_list->thread->process, "New thread joins the current process");
    check(count_threads() == before + 1, "Thread list grows by one and stays valid");

    kill_thread(t);
    wait_thread_gone(id);
    check(count_threads() == before, "Killed thread is removed from the list");
}

void test_thread_run()
{
    g_flag = 0;
    Thread *t = create_thread(worker_set_flag);
    if (t == null) return;
    unsigned int id = t->id;

    for (int i = 0; i < 20; i++) yield;
    check(g_flag == 0, "Thread does not run before start_thread");

    start_thread(t);
    check(wait_for(&g_flag, 1), "Thread runs after start_thread");
    check(wait_thread_gone(id), "Thread is removed after it finishes");
}

void test_thread_kill()
{
    g_stage = 0;
    Thread *t = create_thread(worker_spin);
    if (t == null) return;
    unsigned int id = t->id;

    start_thread(t);
    wait_for(&g_stage, 1);
    check(kill_thread(t) == 1 && wait_thread_gone(id), "A running thread can be killed");
    check(kill_thread(get_thread_by_id(0)) == 0, "The main kernel thread cannot be killed");
}

// ==========    Locks    ==========

void test_lock_recursive()
{
    Lock_t *lock = create_lock();
    if (lock == null) return;

    Lock(lock);
    Lock(lock); // לא אמורה להיות תקיעה בנעילה כפולה
    check(lock->recursive_lock_count == 2, "Owner can lock twice");

    Unlock(lock);
    check(lock->lock_count == 1, "One Unlock keeps the lock held");

    Unlock(lock);
    check(lock->lock_count == 0, "Second Unlock releases the lock");

    free_lock(lock);
}

void test_lock_contention()
{
    g_lock = create_lock();
    if (g_lock == null) return;
    g_stage = 0;
    Lock(g_lock);

    Thread *t = create_thread(worker_lock_once);
    if (t == null) return;
    unsigned int id = t->id;
    start_thread(t);

    wait_for(&g_stage, 1);
    for (int i = 0; i < 20; i++) yield;
    check(g_stage == 1 && t->state == BLOCKED, "Second thread blocks while the lock is held");

    Unlock(g_lock);
    check(wait_for(&g_stage, 2), "Blocked thread gets the lock after Unlock");

    wait_thread_gone(id);
    free_lock(g_lock);
}

void test_lock_mutual_exclusion()
{
    g_lock = create_lock();
    if (g_lock == null) return;
    g_counter = 0;
    g_done = 0;

    for (int i = 0; i < LOCK_WORKERS; i++)
        start_thread(create_thread(worker_locked_increment));

    check(wait_for(&g_done, LOCK_WORKERS), "All workers finish (no deadlock)");
    check(g_counter == LOCK_WORKERS * LOCK_ITER, "No increments are lost under the lock");

    free_lock(g_lock);
}

// ==========    Processes    ==========

void test_process_create()
{
    Process *me = th_list->thread->process;

    Thread *t = create_processe(worker_set_flag);
    check(t != null && t->process != null, "create_processe returns a thread with a process");
    if (t == null || t->process == null) return;

    Process *p = t->process;
    long id = p->id;

    check(p->state == ALIVE && p->parent == me, "New process is ALIVE and its parent is the creator");
    check(p->threads->Len == 1, "New process has one thread");
    check(p->pde_physical_address != 0 && p->pde_physical_address != me->pde_physical_address,
          "New process has its own page directory");
    check(get_process_by_id(id) == p, "get_process_by_id finds the process");
    check(kill_process(kernel_processe) == 0, "The kernel process cannot be killed");

    start_thread(t);
    join_thread(t);
}

void test_process_frames_returned()
{
    unsigned int frames_before = get_free_frames_count();

    Thread *t = create_processe(worker_set_flag);
    if (t == null || t->process == null) return;
    long id = t->process->id;

    start_thread(t);
    wait_process_gone(id);
    check(get_free_frames_count() + LEAK_TOLERANCE >= frames_before,
          "Freeing a process returns its physical frames");
}

void test_process_two_threads()
{
    g_stage = 0;
    g_result = 0;
    g_flag = 0;

    Thread *t = create_processe(worker_spawn_child);
    if (t == null || t->process == null) return;
    long id = t->process->id;
    start_thread(t);

    check(wait_for(&g_stage, 1) && g_result == 1, "A new thread joins the same process");
    check(wait_for(&g_flag, 1), "The extra thread runs");

}

void test_process_isolation()
{
    g_stage = 0;
    g_result = 0;

    Thread *a = create_processe(worker_isolation_a);
    Thread *b = create_processe(worker_isolation_b);
    if (a == null || b == null || a->process == null || b->process == null)
        return;
    long id_a = a->process->id;
    long id_b = b->process->id;

    start_thread(a);
    start_thread(b);

    check(wait_for(&g_stage, 3) && g_result == 1,
          "Same virtual address holds a different value in each process");

    wait_process_gone(id_a);
    wait_process_gone(id_b);
}

// ==========    הרצה    ==========

void _run_thread_process_tests()
{
    passed = 0;
    failed = 0;

    printf("\n========== THREAD & PROCESS TESTS ==========\n");

    test_thread_create();
    test_thread_run();
    test_thread_kill();

    test_lock_recursive();
    test_lock_contention();
    test_lock_mutual_exclusion();

    test_process_create();
    test_process_frames_returned();
    test_process_two_threads();
    test_process_isolation();

    printf("\n========== THREAD & PROCESS SUMMARY ==========\n");
    printf("Passed: %d / %d\n", passed, total_tests);
    printf("Failed: %d\n", failed);
    printf("==============================================\n");
    g_runner_done = 1;
}

void run_thread_process_tests()
{
    g_runner_done = 0;
    Thread *t = create_processe(_run_thread_process_tests);
    start_thread(t);

    while (!g_runner_done) yield;
}