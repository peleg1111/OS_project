[bits 32]
extern key_board_handler

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

    call key_board_handler 

    ; שחזור סגמנטים מקוריים
    pop gs
    pop fs
    pop es
    pop ds

    popa                ; שחזור כל הרגיסטרים
    iretd               ; חזרה מהפסיקה




global divide_by_zero_wrapper
extern exception_Division_By_Zero

divide_by_zero_wrapper:
    push 0 ; dammy error code
    push 0 ; מספר הפסיקה - devide by zero
    pushad
    push esp
    call exception_Division_By_Zero
    add esp, 4  ; stackשחרור הכתובת של ה
    popad
    add esp, 8  ; שחרור הכתובת של מספר הפסיקה וקוד השגיאה
    iretd



global page_fault_wrapper
extern page_fault_handler

page_fault_wrapper:
    pusha                   ; שומר EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10            ; Kernel Data Segment
    mov ds, ax
    mov es, ax

    push esp
    call page_fault_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa 
    add esp, 4
    iretd


global general_protection_fault_wrapper
extern general_protection_fault_handler

general_protection_fault_wrapper:
    push 13
    pushad
    push esp

    call general_protection_fault_handler

    add esp, 4
    popad
    add esp, 8

    iretd

global invalid_opcode_wrapper
extern invalid_opcode_handler

invalid_opcode_wrapper:
    push 0
    push 6
    pushad
    push esp

    call invalid_opcode_handler

    add esp, 4
    popad
    add esp,8

    iretd


global double_fault_wrapper
extern double_fault_handler

double_fault_wrapper:
    push 8
    pushad
    push esp

    call double_fault_handler

    add esp, 4
    popad
    add esp, 8
    iretd