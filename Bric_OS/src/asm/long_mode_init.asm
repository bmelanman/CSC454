global long_mode_start
global reload_segments

extern kernel_main

global ist1
global ist2
global ist3

%define GDT_KMODE_CODE_OFFSET 0x08
%define GDT_KMODE_DATA_OFFSET 0x10

section .text
bits 64
long_mode_start:
    ; load 0 into all data segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; print `OKAY` to screen
    ;mov rax, 0x2f592f412f4b2f4f
    ;mov qword [0xb8000], rax
    ;hlt

    ; run the kernel
    call kernel_main

    hlt

reload_segments:
    ; Reload CS register containing code selector
    push GDT_KMODE_CODE_OFFSET
    lea rax, [rel .reload_CS]
    push rax
    retfq
.reload_CS:
    ; Reload data segment registers
    mov ax, GDT_KMODE_DATA_OFFSET
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

section .bss
    resb 4096
ist1:
    resb 4096
ist2:
    resb 4096
ist3:
    resb 4096