#include "memory_test.h"
#include "stdio.h"
#include "memory.h"
#include "physical_memory.h"
#include "thread.h"
#include "VM_area.h"


#define TEST_ADDR 0xE1000000
#define TEST_PAGE_SIZE 4096

  int tests_passed = 0;
  int tests_failed = 0;

#define CHECK(condition, name) do {        \
    if (condition) {                        \
        tests_passed++;                      \
        printf("[PASS] %s\n", name);          \
    } else {                                   \
        tests_failed++;                         \
        printf("[FAIL] %s\n", name);             \
    }                                             \
} while (0)

void test_summary()
{
    printf("\n========== MEMORY TEST SUMMARY ==========\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("=========================================\n");
}

// ==========    הקצאת פריימים פיזיים    ==========

void test_frame_allocation()
{
    int before = get_free_frames_count();

    unsigned int a = alloc_frame();
    unsigned int b = alloc_frame();
    unsigned int c = alloc_frame();
    //בודק האם ההקצאה עבדה
    CHECK(a != null && b != null && c != null,
          "Allocate three physical frames");
    //בודק האם הפריימים שונים זה מזה
    CHECK(a != b && a != c && b != c,
          "Allocated frames are diferent");
    //בודק האם הם מיושרים
    CHECK((a & 0xFFF) == 0 &&
          (b & 0xFFF) == 0 &&
          (c & 0xFFF) == 0,
          "Physical addresses are page aligned");
    //bit_map בדיקה הקצאה ידנית ב 
    CHECK(is_used(a / TEST_PAGE_SIZE) &&
          is_used(b / TEST_PAGE_SIZE) &&
          is_used(c / TEST_PAGE_SIZE),
          "Allocated frames are occupied");
    //החזרת הזיכרון
    set_bitmap(a / TEST_PAGE_SIZE, 0);
    set_bitmap(b / TEST_PAGE_SIZE, 0);
    set_bitmap(c / TEST_PAGE_SIZE, 0);

    //בודק שהזיכרון הוחזר
    CHECK(!is_used(a / TEST_PAGE_SIZE) &&
          !is_used(b / TEST_PAGE_SIZE) &&
          !is_used(c / TEST_PAGE_SIZE),
          "Released frames are free");
    // בדיקה ידנית שהזיכרון שוחרר
    CHECK(get_free_frames_count() == before,
          "Frame allocation and release restore free count");
}

void test_bitmap_boundaries()
{
    unsigned int frame = memory_bitmap_size * 32 - 1;
    //בודק הקצאה של הפריים האחרון
    set_bitmap(frame, 1);
    CHECK(is_used(frame), "Last valid bitmap frame");

    //בודק את השחרור שלו
    set_bitmap(frame, 0);
    CHECK(!is_used(frame), "Release last valid bitmap frame");

    //בדיקה של איבר מחוץ לגבולות
    CHECK(is_used(memory_bitmap_size * 32),
          "Out of range frame is treated as occupied");
}

void test_frame_reuse()
{
    unsigned int a = alloc_frame();
    //בודק הקצאת זיכרון מוצלחת
    if (a == null) {
        CHECK(0, "Frame reuse test has available memory");
        return;
    }

    //בודק האם יש שימוש חוזר של זיכרון ששוחרר
    unsigned int frame = a / TEST_PAGE_SIZE;
    set_bitmap(frame, 0);

    unsigned int b = alloc_frame();

    CHECK(b == a, "Released lowest available frame is reused");

    if (b != null)
        set_bitmap(b / TEST_PAGE_SIZE, 0);
}

// ======   kernel heap הקצאת זיכרון ב    ======

void test_kmalloc_basic()
{
    unsigned char *p = kmalloc(64);
    // בודק האם ההקצאה היית מוצלחת
    CHECK(p != null, "kmalloc allocates a small block");

    if (p == null)
        return;

    //בודק האם הכתיבה לזיכרון עובדת
    for (int i = 0; i < 64; i++)
        p[i] = (unsigned char)(i ^ 0x5A);

    int valid = 1;

    for (int i = 0; i < 64; i++) {
        if (p[i] != (unsigned char)(i ^ 0x5A)){
            valid = 0;
        }
    }
    CHECK(valid, "Kernel heap saved written data");

    free(p);
}

void test_kmalloc_zero_size()
{
    //בודק הקצאה בגודל של 0
    CHECK(kmalloc(0) == null,
          "kmalloc rejects zero sized allocation");
}

void test_heap_reuse_and_coalescing()
{
    unsigned char *a = kmalloc(128);
    unsigned char *b = kmalloc(256);
    unsigned char *c = kmalloc(128);

    CHECK(a != null && b != null && c != null,
          "Allocate adjacent kernel heap blocks");

    if (!a || !b || !c) {
        if (a) free(a);
        if (b) free(b);
        if (c) free(c);
        return;
    }
    // של המשתנה heap header בודק את השיחרור בעזרת ה  
    Heap_header *bh = (Heap_header *)((unsigned int)b - ALIGN8(sizeof(Heap_header)));
    unsigned int _b = (unsigned int)b;
    free(b);

    CHECK(bh->is_free, "Freed middle block is free");
    //בדיקה של שימוש מחדש בזיכרון ששוחרר
    unsigned char *replacement =
        (unsigned char *)kmalloc(256);

    CHECK(replacement != null && (unsigned int)replacement == _b,
          "Allocator reuses a sufficiently large free block");

    free(a);
    free(c);

    if (replacement)
        free(replacement);
}

void test_heap_data_isolation()
{
    unsigned char *a = kmalloc(96);
    unsigned char *b = kmalloc(96);
    // בודק האם ההקצאה עבדה
    CHECK(a != null && b != null && a != b,
          "Separate allocations have distinct addresses");

    if (!a || !b) {
        if (a) free(a);
        if (b) free(b);
        return;
    }
    // בדיקת כתיבה לזיכרון
    memset(a, 0xAA, 96);
    memset(b, 0x55, 96);

    int valid = 1;

    for (int i = 0; i < 96; i++) {
        if (a[i] != 0xAA || b[i] != 0x55)
            valid = 0;
    }

    CHECK(valid, "Adjacent allocations do not corrupt each other");

    free(a);
    free(b);
}

void test_heap_stress()
{

    //בדיקת תקינות של הקצאה בגדלים שונים
    #define N 24
    void *blocks[N];
    unsigned int sizes[N];

    int valid = 1;

    for (int i = 0; i < N; i++) {
        sizes[i] = 16 + (i * 37);
        blocks[i] = kmalloc(sizes[i]);

        if (blocks[i] == null) {
            valid = 0;
            break;
        }
        memset(blocks[i], i + 1, sizes[i]);
    }

    CHECK(valid, "Repeated allocations of different sizes");

    // בדיקה של הכתיבה לזיכרון
    for (int i = 0; i < N; i++) {
        if (blocks[i] == null)
            continue;

        unsigned char *p = blocks[i];

        for (unsigned int j = 0; j < sizes[i]; j++) {
            if (p[j] != (unsigned char)(i + 1)) {
                valid = 0;
                break;
            }
        }
    }

    CHECK(valid, "Stress test saved every allocated block");

    for (int i = N - 1; i >= 0; i--) {
        if (blocks[i] != null)
            free(blocks[i]);
    }
}

// =====    Paging && unmapping    =====

void test_paging_basic()
{
    //בדיקת מיפוי כתובות
    unsigned int before = get_free_frames_count();

    int result = map_page(TEST_ADDR, 0);

    CHECK(result == 1, "map_page maps a new virtual page");
    //בדיקת כתיבה לכתובת שעברה מיפוי
    if (!result)
        return;

    volatile unsigned int *p =
        (volatile unsigned int *)TEST_ADDR;

    *p = 0x1234ABCD;

    CHECK(*p == 0x1234ABCD,
          "Mapped page supports read and write");

    //בדיקת גבולות של טבלת הדפים לאחר מיפוי
    unmap(TEST_ADDR);

    CHECK(!is_used(0xFFFFFFFF / TEST_PAGE_SIZE),
          "Bitmap boundary remains safe after unmapping");
    //בדיקה שהשחרור עובד
    // סיבה: צריך להקצות פריים גם לטבלה עצמה של הדפים כל פעם שגומרים טבלת דפים קיימת(before - 1)
    CHECK(get_free_frames_count() >= before-1,
          "Unmapping releases the mapped data frame");

}

void test_paging_multiple_pages()
{
    //בדיקה של מיפוי מספר עמודים
    unsigned int a = TEST_ADDR + 0x01000000;
    unsigned int b = a + TEST_PAGE_SIZE;
    unsigned int c = b + TEST_PAGE_SIZE;

    int ra = map_page(a, 0);
    int rb = map_page(b, 0);
    int rc = map_page(c, 0);

    CHECK(ra && rb && rc,
          "Map three virtual pages");
    // בדיקה של כתיבה לזיכרון ממופה
    if (ra && rb && rc) {
        volatile unsigned int *pa = (volatile unsigned int *)a;
        volatile unsigned int *pb = (volatile unsigned int *)b;
        volatile unsigned int *pc = (volatile unsigned int *)c;

        *pa = 0x11111111;
        *pb = 0x22222222;
        *pc = 0x33333333;

        CHECK(*pa == 0x11111111 &&
              *pb == 0x22222222 &&
              *pc == 0x33333333,
              "pages hold independent data");
    }

    if (ra) unmap(a);
    if (rb) unmap(b);
    if (rc) unmap(c);
}

void test_paging_unmap_twice()
{
    //בדיקה שאין שגיאה לאחר הסרת מיפוי פעמיים ברצף מאותה כתובת
    unsigned int addr = TEST_ADDR + 0x02000000;

    int mapped = map_page(addr, 0);

    CHECK(mapped, "Map page for repeated unmap test");

    if (!mapped)
        return;

    unmap(addr);
    unmap(addr);

    CHECK(1, "Repeated unmap does not crash");
}

// =====    Virtual memory areas    =====

void test_vma_lookup()
{
    //בדיקה שאין התנגשות בין איזורי זיכרון ווירטואליים
    VM_area *vma = null;

    int a = add_vma(&vma, 0x10000000, 0x10004000, 0x7, "test_a");

    int b = add_vma(&vma, 0x10004000, 0x10008000, 0x7, "test_b");

    CHECK(a && b, "Create non overlapping VMAs");
    //בדיקת חיפוש של איזור זיכרון ווירטואלי
    CHECK(find_vma(vma, 0x10000000) != null,
          "Find VMA at its first address");

    CHECK(find_vma(vma, 0x10003FFF) != null,
          "Find VMA at its last included address");
    //בדיקת חיפוש לפי הגבולות של איזור הזיכרון
    CHECK(find_vma(vma, 0x10004000) != null,
          "Adjacent VMA boundary resolves correctly");

    CHECK(find_vma(vma, 0x10008000) == null,
          "VMA end address is excluded");
    //בדיקה שאין חפיפה בין איזורים לאחר ניסיון הוספה
    CHECK(!add_vma(&vma, 0x10002000, 0x10006000, 0x7, "overlap"), "Reject overlapping VMA");

    CHECK(find_vma(vma, 0x20000000) == null,
          "Lookup outside all VMAs returns null");

    while (vma != null) {
        VM_area *next = vma->next;
        free(vma);
        vma = next;
    }
}

// =====    malloc/realloc    =====

void test_user_heap_if_available()
{
    //אם יש זמינות
    if (!th_list || !th_list->thread ||
        !th_list->thread->process) {
        printf("[SKIP] User heap tests: no process context\n");
        return;
    }
    // בדיקת הקצאת זיכרון במרחב המשתמש
    unsigned char *p = malloc(128);

    CHECK(p != null, "malloc allocates in current process");

    if (!p)
        return;
    //בדיקת קריא וכתיבה לזיכרון
    memset(p, 0x3C, 128);

    int valid = 1;
    for (int i = 0; i < 128; i++) {
        if (p[i] != 0x3C)
            valid = 0;
    }

    CHECK(valid, "User heap data is writable and readable");
    // בדיקת הגדלה של זיכרון
    unsigned char *q = realloc(p, 512);

    CHECK(q != null, "realloc grows an allocation");
    // בודק האם ההגדלה של הזיכרון שומר נתונים ישנים
    if (q) {
        valid = 1;

        for (int i = 0; i < 128; i++) {
            if (q[i] != 0x3C)
                valid = 0;
        }

        CHECK(valid, "realloc preserves old data");

        free(q);
    }
    else {
        free(p);
    }
}


void _run_memory_tests()
{
    
    tests_passed = 0;
    tests_failed = 0;

    printf("\n========== MEMORY TESTS ==========\n");

    test_frame_allocation();
    test_bitmap_boundaries();
    test_frame_reuse();

    test_kmalloc_basic();
    test_kmalloc_zero_size();
    test_heap_reuse_and_coalescing();
    test_heap_data_isolation();
    test_heap_stress();

    test_paging_basic();
    test_paging_multiple_pages();
    test_paging_unmap_twice();

    test_vma_lookup();
    test_user_heap_if_available();

    test_summary();
}

void run_memory_tests(){
    Thread* t = create_processe(_run_memory_tests);
    start_thread(t);
    join_thread(t);
}