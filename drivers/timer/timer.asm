[bits 32]


global timer_wrapper
extern timer_handler
timer_wrapper:

    pushad          ; שמירת EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    call timer_handler
    popad           ; שחזור רגיסטרים כלליים

    iretd