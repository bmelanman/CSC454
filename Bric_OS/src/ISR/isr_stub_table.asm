global isr_stub_table

extern exception_handler
extern interrupt_handler

%macro isr_err_stub 1
isr_stub_%+%1:
    call exception_handler
    iretq
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push %1 ; Arg 1: ISR number
    pop rdi

    push 0 ; Arg 2: Error code
    pop rsi

    call interrupt_handler
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