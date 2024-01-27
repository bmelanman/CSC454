%define GDT_OFFSET_KMODE_CODE_SEG 0x08
%define GDT_OFFSET_KMODE_DATA_SEG 0x10

extern exception_handler

%macro isr_err_stub 1
isr_stub_%+%1:
    call exception_handler
    iretq
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    call exception_handler
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

global isr_stub_table

isr_stub_table:
%assign i 0 
%rep    32 
    dq isr_stub_%+i
%assign i i+1 
%endrep



global reload_segments
reload_segments:
   ; Reload CS register:
   push GDT_OFFSET_KMODE_CODE_SEG   ; Push code segment to stack
   lea rax, [rel .reload_CS]        ; Load address of .reload_CS into RAX
   push rax                         ; Push this value to the stack
   retfq                            ; Perform a far return to reload CS
.reload_CS:
   ; Reload data segment registers
   mov   ax, GDT_OFFSET_KMODE_DATA_SEG
   mov   ds, ax
   mov   es, ax
   mov   fs, ax
   mov   gs, ax
   mov   ss, ax
   ret

;//global   isr_wrapper
;
;//extern   interrupt_handler
;
;//section .text
;//bits 32
;//isr_wrapper:
;//    pushad
;
;//    ; Note: C code following the sysV ABI requires
;//    ;       DF to be clear on function entry
;//    cld
;
;//    call interrupt_handler
;
;//    popad
;//    iret