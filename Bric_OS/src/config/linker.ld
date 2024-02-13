ENTRY(start)

SECTIONS
{
    . = 1M;

    .boot :
    {
        /* Kernel Starting Address */
        kernel_start = .;

        /* Place the multiboot header at the beginning */
        *(.multiboot)

        *(.boot)
    }

    .text :
    {
        *(.text)
    }

    .rodata ALIGN (0x1000) :
    {
        *(.rodata)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)

        /* Kernel Ending Address, aligned to 4k */
        . = ALIGN(0x1000);
        kernel_end = .;
    }
}