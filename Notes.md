# OS2 Notes

## Serial I/O

### Serial Input

FIFO Buffer of input characters:

- Consumer reads from FIFO as needed/able
- Producer writes to buffer from an ISR

### Serial Output

FIFO buffer of output characters

- Producer writes to FIFO as needed/able
- Consumer reads from FIFO from an ISR

### Configuration

- Use 8 bits of data, no parity bit, 1 stop bit (a.k.a. '8N1')
- Enable serial IO interrupt(s)
- Unmask UART on the PIC

## System Memory, Page Tables, and Page Allocators

### How do we efficiently allocate virtual memory?

- Demand paging: Allocate pages at a time (rather than in bytes or otherwise), and only pre-allocate in-use pages.

Say a process calls `malloc(0x2000)` (8192 bytes).

- Find available **physical** memory to put the requested data.
- Allocate the first page frame (only 4096 bytes) and return the address.
- Only allocate requested pages, so if a page fault occurs, only then will we allocate more space.

## Multiboot 2

Multiboot2 passes information about the underlying computer using tags. These tags contain two pieces of valuable information:
The region(s) of physical memory present in the machine.

The memory locations occupied by your kernel's text and data segments.

### Tag Header and Types

Tags start at address in `%ebx`

All tags start with a 4 byte `type` field, and a 4 byte `size` field. The remaining content varies depending on the given `type`.

Notable types:

- Type 4: Basic Memory Info.
- Type 6: Memory Map Info.
    - This entry would also contain an array of memory info entries.
- Type 9: ELF Symbols
    - You need to inspect each ELF64 header, looking at the in-memory segment address and length. These are the memory regions that contain your kernel text and data.
    - **AVOID ALLOCATING PAGES FROM THOSE MEMORY REGIONS!!!**

## Virtaul Address Maping

Recommendations from the Big Man himself:

- Keep a 1:1 mapping from address `0x0000000000` to `0x8000000000` ( 512 GB ) at the beginning of the virtual address layout.

    - This also means we don't have to relocate the kernel!

- Reserve the first 16 blocks for the kernel



```text
Virtaul Memory Layout

[-----------------------------------] 0x0000000000000000
[ Entry 0: Kernel (1:1 Mapping)     ] Kernel Space Memory Start
[-----------------------------------] 0x0000008000000000
[ Entry 1: Kernel Heap              ]
[-----------------------------------] 0x0000010000000000
[ Entry 2: ...                      ]
...
[ Entry 14: ...                     ]
[-----------------------------------] 0x0020000000000000
[ Entry 15: Kernel Stack            ] Kernel Space Memory End
[-----------------------------------] 0x0040000000000000
[ Entry 16: ...                     ] User Space Memory Start
...
[ Entry 31:                         ] User Space Memory End
[-----------------------------------] 0xFFFFFFFFFFFFFFFF
```

## Context Swapping

```c
struct reg_cxt
{
    uint64_t rsp;
    uint64_t rflags;
    uint64_t rip;
    uint64_t cs;
    uint64_t ss;
    ...
    uint64_t fs;
    uint64_t gs;
    uint64_t rax;
    uint64_t rbx;
    ...
    uint64_t r8;
    uint64_t r9;
    ...
    uint64_t r14;
    uint64_t r15;
};
```

### Required Registers to Save/Resore

**NOTE:** Floating point is not required, but saving all FP registers would be required if implemented.

- RBP, RSP, RIP, RFLAGS
- CS, SS, DS, ES, FS, GS
- RAX, RBX, RCX, RDX, RDI, RSI
- R8, R9, R10, R11, R12, R13, R14, R15


### Process Swapping

- Context should be saved at the **END** of an interrupt.

- Keep two global variables, one for the current process and one for the next process.

- The scheduler simply sets the next process global.

- Leverage your existing interrupt handler to perform the stack switch. After the C interrupt dispatch function returns to the assembly portion, check if the next process is different than the current process. If so save all the registers into the current process's context, load all the next process's context values (note that some are on the stack), update the current process global, update the next process global, and then let the code continue on to call "iretq".

- This arrangement permits a context switch on every interrupt, assuming the scheduler picks a different process.

- Both the exit and yield implementations are small since they are already using traps. For instance, yield simply calls the reschedule function.

- PROC_run can just set the current process global to a pointer to the statically allocated main process's context and call yield.

- Keep your scheduler simple to start with. Round robin is a good choice.

### Cooperative VS Preemptive

- Cooperative Scheduling:
    - Call `yield` to pass priority to another thread.

- Preemptive Scheduling:
    - Use a timer to trigger an interrupt, which then passes priority too another thread
    - Has the potential to create MANY race conditions.

## Block Devices

Ideally, an Object-Oriented programming language would be preferred to allow the use of classes and interfaces like so:

```c++
class BlockDevice
{
   public:
      virtual int read_block(uint64_t blk_num, void *dst) = 0;
      virtual int write_block(uint64_t blk_num, void *src) = 0;
      uint32_t blk_size;
      uint64_t tot_length;
}

class ATABlockDevice : BlockDevice
{
   public:
      int read_block(uint64_t blk_num, void *dst);
      int write_block(uint64_t blk_num, void *src);
}
```