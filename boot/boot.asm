[bits 16]
[org 0x7c00]

KERNEL_SEG    equ 0x1000 
KERNEL_OFFSET equ 0x10000 
CHUNK equ 32 ; סקטורים בכל קריאה 

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov bp, 0x9000 ; הגדרת מחסנית
    mov sp, bp
    sti

    mov [BOOT_DRIVE], dl 

    call load_kernel
    call switch_to_pm
    jmp $

; טעינת הקרנל בחלקים עם INT 13h
load_kernel:
    mov word [dap_seg], KERNEL_SEG
    mov word [dap_lba], 1 ; ( boot loader אחרי ה )מתחילים מסקטור 1
    mov bx, SECTORS_TO_LOAD       ; כמה סקטורים נשארו לקרוא

.loop:
    mov ax, CHUNK
    cmp bx, ax
    jae .go
    mov ax, bx ; CHUNK החלק האחרון קטן מ 
.go:
    mov [dap_count], ax
    push ax
    mov si, dap
    mov dl, [BOOT_DRIVE]
    mov ah, 0x42
    int 0x13
    jc disk_error
    pop ax

    sub bx, ax
    add [dap_lba], ax
    shl ax, 5 ; 32 sectors = 16KB
    add [dap_seg], ax
    test bx, bx
    jnz .loop
    ret

disk_error:
    mov dl, ah ; שומרים את קוד השגיאה
    xor bx, bx
    mov ah, 0x0e
    mov al, 'E'
    int 0x10

    mov al, dl
    shr al, 4
    call print_nibble
    mov al, dl
    and al, 0x0f
    call print_nibble
    jmp $

print_nibble:
    add al, '0'
    cmp al, '9'
    jbe .print
    add al, 7 ; A-F
.print:
    mov ah, 0x0e
    int 0x10
    ret

; Disk Address Packet
align 4
dap:
    db 0x10, 0
dap_count: dw 0
dap_off:   dw 0
dap_seg:   dw 0
dap_lba:   dq 0

; GDT 
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

    mov ecx, SECTORS_TO_LOAD * 128 ; 512 / 4 = 128


    mov esi, KERNEL_OFFSET ; הכתובת שממנה מעתיקים את הקרנל
    mov edi, 0x100000  ; הכתובת שאליה מעתיקים את הקרנל

    cld
    rep movsd  ; העתקת הקרנל

    call 0x100000
    jmp $

BOOT_DRIVE db 0
times 510-($-$$) db 0
dw 0xaa55