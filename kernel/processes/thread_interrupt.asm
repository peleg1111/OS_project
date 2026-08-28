[bits 32]

extern switch_thread
extern get_cr3
global switch_thread_wrapper

switch_thread_wrapper:
    pushad
    push esp

    call switch_thread
    add esp, 4
    mov esp, eax

    call get_cr3
    cmp eax, 0
    jz skip_cr3
    mov ecx, cr3
    cmp eax, ecx
    jz skip_cr3
    mov cr3, eax

skip_cr3:
    mov ax, [esp + 36]    ; CS שליפת 
    and ax, 3             ;0 = Ring 0  /  3 = Ring 3
    cmp ax, 3
    jne load_kernel_segments

load_user_segments:
    mov ax, 0x23          ; USER_DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp do_restore

load_kernel_segments:
    mov ax, 0x10          ; KERNEL_DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

do_restore:
    popad
    iretd
