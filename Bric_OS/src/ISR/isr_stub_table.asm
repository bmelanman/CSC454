global isr_stub_table

extern exception_handler
extern interrupt_handler

%macro pushaq 0
    ; Save registers (except rsp)
    sub rsp, $120
    mov rax, [rsp+112]
    mov rbx, [rsp+104]
    mov rcx, [rsp+96]
    mov rdx, [rsp+88]
    mov rbp, [rsp+80]
    mov rdi, [rsp+72]
    mov rsi, [rsp+64]
    mov r8,  [rsp+56]
    mov r9,  [rsp+48]
    mov r10, [rsp+40]
    mov r11, [rsp+32]
    mov r12, [rsp+24]
    mov r13, [rsp+16]
    mov r14, [rsp+8]
    mov r15, [rsp+0]
%endmacro

%macro popaq 0
    ; Restore registers (except rsp)
    mov [rsp+0],  r15
    mov [rsp+8],  r14
    mov [rsp+16], r13
    mov [rsp+24], r12
    mov [rsp+32], r11
    mov [rsp+40], r10
    mov [rsp+48], r9
    mov [rsp+56], r8
    mov [rsp+64], rsi
    mov [rsp+72], rdi
    mov [rsp+80], rbp
    mov [rsp+88], rdx
    mov [rsp+96], rcx
    mov [rsp+104], rbx
    mov [rsp+112], rax
    add rsp, $120
%endmacro

%macro isr_err_stub 1
isr_stub_%+%1:
    ; Exception handler won't be returning,
    ; so we don't need to save any registers
    call exception_handler
    iretq
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    ; Save registers
    pushaq
    ; Clear direction flag
    cld

    ; Set up ISR stack frame
    push %1 ; Arg 1: ISR number
    pop rdi
    push 0  ; Arg 2: Error code
    pop rsi

    ; Call the interrupt handler
    call interrupt_handler

    ; Restore registers
    popaq

    iretq
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

%assign i 32
%rep    224
    isr_no_err_stub i
%assign i i+1
%endrep

isr_stub_table:
%assign i 0
%rep    256
    dq isr_stub_%+i
%assign i i+1
%endrep