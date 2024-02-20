global isr_stub_table

extern interrupt_handler
extern reload_segments

extern ist1
extern ist2
extern ist3
extern ist4

%macro push_args 0
    push rdi
    push rsi
    push rdx
%endmacro

%macro pop_args 0
    pop rdx
    pop rsi
    pop rdi
%endmacro

%macro pop_nonvolatile 0
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    ; Skip rsp
    pop rsi
    pop rdi
    pop rbx
%endmacro

%macro pushaq 0
    ; Save registers (except rsp)
    push rax
    push rbx
    push rcx
    ;push rdx
    ;push rdi
    ;push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push rbp
    ;push rsp
    ;push cs
    ;push ss
    ;push ds
    ;push es
    ;push fs
    ;push gs
%endmacro

%macro popaq 0
    ; Restore registers (except rsp)
    ;pop gs
    ;pop fs
    ;pop es
    ;pop ds
    ;pop ss
    ;pop cs
    ;pop rsp
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    ;pop rsi
    ;pop rdi
    ;pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

; Interrupt Frame Format
; +-------------------------+
; | Error Code (4 Bytes)    | (+0x00)
; +-------------------------+
; | RIP (8 Bytes)           | (+0x08)
; +-------------------------+
; | CS (2 Bytes)            | (+0x10)
; +-------------------------+
; | RFLAGS (8 Bytes)        | (+0x18)
; +-------------------------+
; | RSP (8 Bytes)           | (+0x20)
; +-------------------------+
; | SS (2 Bytes)            | (+0x28)
; +-------------------------+

%define NO_ERR 0
%define HAS_ERROR_CODE 1

%macro isr_stub 2
isr_stub_%+%1:
    ; Save argument registers
    push_args

    ; Load the interrupt number into Arg 1
    mov rdi, %1

    ; Load the error code into Arg 2 (if present)
%if %2 == HAS_ERROR_CODE
    mov rsi,[rsp+24] ; Error code
%else
    mov rsi, 0 ; No error code
%endif

    ; Save the remaining registers
    pushaq
    ; Clear direction flag
    cld

    ; Call the exception handler
    call interrupt_handler

    ; Restore registers
    popaq
    pop_args

    ; Remove the error code from the stack if present
%if %2 == HAS_ERROR_CODE
    add rsp, 8
%endif

    ; Return from the interrupt
    iretq

%endmacro

; Exception Interrupts (0x00)
isr_stub 0,  NO_ERR
isr_stub 1,  NO_ERR
isr_stub 2,  NO_ERR
isr_stub 3,  NO_ERR
isr_stub 4,  NO_ERR
isr_stub 5,  NO_ERR
isr_stub 6,  NO_ERR
isr_stub 7,  NO_ERR
isr_stub 8,  HAS_ERROR_CODE
isr_stub 9,  NO_ERR
isr_stub 10, HAS_ERROR_CODE
isr_stub 11, HAS_ERROR_CODE
isr_stub 12, HAS_ERROR_CODE
isr_stub 13, HAS_ERROR_CODE
isr_stub 14, HAS_ERROR_CODE
isr_stub 15, NO_ERR
isr_stub 16, NO_ERR
isr_stub 17, HAS_ERROR_CODE
isr_stub 18, NO_ERR
isr_stub 19, NO_ERR
isr_stub 20, NO_ERR
isr_stub 21, NO_ERR
isr_stub 22, NO_ERR
isr_stub 23, NO_ERR
isr_stub 24, NO_ERR
isr_stub 25, NO_ERR
isr_stub 26, NO_ERR
isr_stub 27, NO_ERR
isr_stub 28, NO_ERR
isr_stub 29, NO_ERR
isr_stub 30, HAS_ERROR_CODE
isr_stub 31, NO_ERR

; PIC Controller Interrupts (0x20)
isr_stub 32, NO_ERR
isr_stub 33, NO_ERR
isr_stub 34, NO_ERR
isr_stub 35, NO_ERR
isr_stub 36, NO_ERR
isr_stub 37, NO_ERR
isr_stub 38, NO_ERR
isr_stub 39, NO_ERR

; PIC Peripheral Interrupts (0x28)
isr_stub 40, NO_ERR
isr_stub 41, NO_ERR
isr_stub 42, NO_ERR
isr_stub 43, NO_ERR
isr_stub 44, NO_ERR
isr_stub 45, NO_ERR
isr_stub 46, NO_ERR
isr_stub 47, NO_ERR

isr_stub_table:
%assign i 0
%rep    48
    dq isr_stub_%+i
%assign i i+1
%endrep

section .bss
align 4096
ist_end:
    resb 4096
    resb 4096
    resb 4096
ist4:
    resb 4096
    resb 4096
    resb 4096
ist3:
    resb 4096
    resb 4096
    resb 4096
ist2:
    resb 4096
    resb 4096
    resb 4096
ist1:
    resb 4096


;%macro push_volatile 0
;    push rax
;    push rcx
;    push rdx
;    push r8
;    push r9
;    push r10
;    push r11
;%endmacro

;%macro pop_volatile 0
;    pop r11
;    pop r10
;    pop r9
;    pop r8
;    pop rdx
;    pop rcx
;    pop rax
;%endmacro

;%macro push_nonvolatile 0
;    push rbx
;    push rdi
;    push rsi
;    ; Skip rsp
;    push r12
;    push r13
;    push r14
;    push r15
;    push rbp
;%endmacro

;%macro push(%reg)
;    add rsp, -8
;    mov [rsp], %reg
;%endmacro

;%macro pop(%reg)
;    mov %reg, [rsp]
;    add rsp, 8
;%endmacro