[bits 32]

global create_gdt
extern gdt_ptr

create_gdt:
    lgdt [gdt_ptr]

    ;איפוס הסגמנתים של הקרנל
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush

.flush:
    ret