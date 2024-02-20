global long_mode_start

extern kernel_main

; void reload_segments( uint16_t code_offset, uint16_t data_offset )
global reload_segments

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

    ; Pop the multiboot information from the stack and pass it to the kernel
    pop rdi
    pop rsi

    ; run the kernel
    call kernel_main

end:
    hlt
    jmp end

; Reload CS register containing code selector
; Arg 1 (di):  Code selector offset
; Arg 2 (si):  Data selector offset
reload_segments:
    ; Push the code selector offset onto the stack (push 8 bytes even though it's only 2)
    push rdi
    lea rdi, [rel .reload_CS]
    push rdi
    retfq
; Reload data segment registers
.reload_CS:
    ; Load the data selector offset into the data segment registers
    mov ax, si
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret