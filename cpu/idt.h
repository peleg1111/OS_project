#ifndef IDT_H
#define IDT_H

#include "const.h" // בשביל טיפוסים בסיסיים אם צריך

// מבנה של שער ב-IDT
typedef struct {
    unsigned short low_offset; // 16 הביטים הנמוכים של כתובת הפונקציה
    unsigned short sel;        // Kernel Segment Selector
    unsigned char always0;     // תמיד 0
    unsigned char flags;       // דגלים (סוג השער, רמת הרשאה)
    unsigned short high_offset;// 16 הביטים הגבוהים של כתובת הפונקציה
} __attribute__((packed)) idt_gate_t;

// מבנה שמתאר את מיקום הטבלה כולה (בשביל פקודת lidt)
typedef struct {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) idt_register_t;

#define IDT_ENTRIES 256

// הצהרות פונקציות
void set_idt_gate(int n, unsigned int handler);
void load_idt();

#endif