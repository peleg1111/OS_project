#ifndef IDT_H
#define IDT_H

#include "stdio.h"

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


typedef struct {

    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp_dummy; // (לא רלוונטי ) register esp הערך הישן של
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    unsigned int interupt_number; // מספר הפסיקה
    unsigned int err_code;// (dammy / real) קוד השגיאה
    
    unsigned int eip;    // הכתובת שבה המערכת קרסה
    unsigned int cs;     // סגמנט הקוד
    unsigned int flags; // דגלי המעבד

    unsigned int user_esp;
    unsigned int user_ss;
    
} __attribute__((packed)) registers_t;

#define IDT_ENTRIES 256

// הצהרות פונקציות
void set_idt_gate(int n, unsigned int handler, unsigned int flags, unsigned int sel);
void load_idt();
void error_print(registers_t* stack_addr);

#define ring3 0xEE
#define ring0 0x8E

#define divide_by_zero_port 0
#define page_fault_port 14
#define timer_port 32
#define keyboard_port 33
#define switch_thread_port 0x81
#define invalid_opcode_port 6
#define double_fault_port 8
#define general_protection_fault_port 13

#endif