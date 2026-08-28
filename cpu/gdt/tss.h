#ifndef TSS_H
#define TSS_H

#define KERNEL_DATA_SEGMENT 0x10
#define KERNEL_CODE_SEGMENT 0x08
#define USER_DATA_SEGMENT 0x23
#define USER_CODE_SEGMENT 0x1B

#define TSS_SEGMENT 0x28


typedef struct
{
    unsigned int prev_tss;
    unsigned int esp0;     // ראש מחסנית הקרנל
    unsigned int ss0;      // סגמנט הנתונים של הקרנל

    // לא בשימוש - מנוהל על ידי התוכנה
    unsigned int unused[22]; // בתים שמרפדים את שאר אוגרי החומרה
    unsigned short unused2;

    // מגדיר את ההרשאות של התהליך - אם הוא יכול לבצע פעולות קלט/פלט או לא
    unsigned short iomap_base;// מאותחל לגודל מבנה הנתונים הזה כדי למנוע גישה למפת הקלט/פלט

} __attribute__((packed)) Tss_entry;

typedef struct
{
    unsigned int edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;

    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int user_esp;
    unsigned int ss;
} __attribute__((packed)) User_stack_frame;

typedef struct
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) gdt_ptr_t;

void init_gdt();
void tss_init(unsigned short kernel_data_segment, unsigned int kernel_esp);
void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);

extern Tss_entry tss;

#endif  