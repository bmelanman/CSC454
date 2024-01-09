global long_mode_start

extern kernel_main
extern VGA_init

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

    ; init vga
    call VGA_init

    ; run the kernel
    call kernel_main

    hlt