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