[bits 32]
extern isr_handler

global isr_wrapper
isr_wrapper:
    pusha ; שומר EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI

    ; שמירת הסגמנטים הנוכחיים
    push ds
    push es
    push fs
    push gs

    ; איפוס הסגמנטים
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler    ; קריאה ל C handler

    ; שחזור סגמנטים מקוריים
    pop gs
    pop fs
    pop es
    pop ds

    popa                ; שחזור כל הרגיסטרים
    iretd               ; חזרה מהפסיקה