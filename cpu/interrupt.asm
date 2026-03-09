[extern isr_handler] ; הפונקציה מה-const.c שלך

global isr_wrapper
isr_wrapper:
    pusha               ; שומר את כל הרגיסטרים (EAX, EBX וכו')
    push ds             ; שומר סגמנטים
    push es
    push fs
    push gs

    call isr_handler    ; קפיצה לקוד ה-C שלך!

    pop gs              ; מחזיר סגמנטים
    pop fs
    pop es
    pop ds
    popa                ; מחזיר רגיסטרים
    iretd               ; Interrupt Return - חוזר לתוכנית הראשית