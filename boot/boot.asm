[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0xA000

start:
    mov [BOOT_DRIVE], dl

    ; הגדרת מחסנית
    mov bp, 0x9000
    mov sp, bp

    call load_kernel
    call switch_to_pm
    jmp $

load_kernel:
    mov bx, KERNEL_OFFSET 
    mov dh, 50   ; יש להגדיל את הכמות 
    mov dl, [BOOT_DRIVE]
    mov ah, 0x02          
    mov al, dh
    mov ch, 0x00          
    mov dh, 0x00          
    mov cl, 0x02          
    int 0x13
    jc disk_error         
    ret

disk_error:
    mov ah, 0x0e
    mov al, 'E'           
    int 0x10
    jmp $

; --- GDT ---
gdt_start:
    dq 0x0                
gdt_code:
    dw 0xffff, 0x0
    db 0x0, 10011010b, 11001111b, 0x0
gdt_data:
    dw 0xffff, 0x0
    db 0x0, 10010010b, 11001111b, 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

switch_to_pm:
    cli                     
    lgdt [gdt_descriptor]   
    mov eax, cr0
    or eax, 0x1             
    mov cr0, eax
    jmp 0x08:init_pm        

[bits 32]
init_pm:
    mov ax, 0x10            
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    call KERNEL_OFFSET
    jmp $

BOOT_DRIVE db 0
times 510-($-$$) db 0
dw 0xaa55