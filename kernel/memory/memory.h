#ifndef MEMORY_H
#define MEMORY_H

#include "physical_memory.h"

#define memory_bitmap_size 32768 
#define frame_size 4096
#define max_bytes_gap 50
#define pde_addr 0xFFFFF000
#define pte_dir 0xFFC00000 // pde[1023]

#define kb1 0x400      // 1 KB
#define kb4 0x1000     // 4 KB
#define mb1 0x100000   // 1 MB
#define mb4 0x400000   // 4 MB
#define gb1 0x40000000 // 1 GB
#define gb4 gb1 * 4 // 4 GB

#define kernel_limit gb1

// יישור ל 8 בתים
#define ALIGN8(sz) (((sz) + 7) & ~7)


typedef struct {

    unsigned int present : 1;// האם הדף נמצא בזיכרון או לא
    unsigned int read_write : 1;// האם הדף ניתן לכתיבה או רק לקריאה
    unsigned int user_supervisor : 1;// האם הדף ניתן לגישה למשתמש או רק למנהל
    unsigned int write_through : 1;// האם הדף נכתב מיד לדיסק או נשמר בזיכרון המטמון
    unsigned int cache_disabled : 1;// האם הדף ניתן לאחסון במטמון או לא
    unsigned int accessed : 1;// האם המעבד קרא מהדף הזה    
    unsigned int dirty : 1;// האם הדף השתנה(המעבד כתב בו = 1)
    unsigned int page_size : 1; // גודל הדף( 4 = 0KB, 1 = 4MB ( הכתובת תהייה למקום אמיתי בזיכרון ) )
    unsigned int global : 1;// האם הדף גלובלי או לא (אם הוא גלובלי הוא לא יימחק מהמיפוי של הדפים כאשר מעבירים בין תוכניות)
    unsigned int free_bits : 3;// לשימוש אישי שלי
    unsigned int frame : 20; // כתובת הפיזית של הדף 

}__attribute__((packed)) Page_entery_bits;

typedef union{

    Page_entery_bits bits;
    unsigned int value;

} Page_entry;

typedef struct Heap_header
{
    unsigned int is_free : 1;
    unsigned int size;
    struct Heap_header* next;
    struct Heap_header* prev;

} __attribute__((packed, aligned(4))) Heap_header;


typedef struct {

    Heap_header* heap_block;
    unsigned int heap_start;

} Heap_manager;// יש להשתמש בעתיד


struct Thread_list;

extern Page_entry page_table[1024] __attribute__((aligned(4096)));
extern Page_entry page_directory[1024] __attribute__((aligned(4096)));
extern unsigned int pde_physical_address;

void paging_init();
void memory_init();
void* _malloc(unsigned int size, struct Thread_list* th);
void* malloc(unsigned int size);
void* kmalloc(unsigned int size);
void* realloc(void* ptr , unsigned int size);

void __free(void* ptr,struct Thread_list* th);
void _free(void *ptr);

struct VM_area;
int map_page(unsigned int virtual_addr, unsigned int is_user);
void unmap(unsigned int virtual_addr);

void memset(void *ptr, int value, unsigned int size);
void memcopy(void *target, void *source, unsigned int size);
unsigned int calc_heap_size(Heap_header* heap);

extern Heap_header* user_heap_start_block;
extern unsigned int user_heap_start;
extern Heap_header* kernel_heap_start_block;

#define free(ptr)          \
    do                     \
    {                      \
        _free(ptr);        \
        if ((ptr) != null)\
        {\
            (ptr) = null;\
        }\
    } while (0)



#endif