OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS {
    text    PT_LOAD FLAGS(5);
    rodata  PT_LOAD FLAGS(4);
    data    PT_LOAD FLAGS(6);
}

SECTIONS {
    /**************************************************************************
     * Higher-half sections                                                   *
     **************************************************************************/
    . = 0xffffffff80000000;

    g_kernelStart = .;
    g_kernelTextStart = .;

    .text : {
        *(.text)
        *(.text.*)
    } : text

    g_kernelTextEnd = .;

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    g_kernelRodataStart = .;

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } : rodata

    g_kernelRodataEnd = .;

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    g_kernelDataStart = .;

    .data : {
        *(.data)
        *(.data.*)
    } : data

    g_kernelDataEnd = .;

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    g_kernelBssStart = .;

    .bss : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    } : data

    g_kernelBssEnd = .;
    g_kernelEnd = .;

    /*
    /DISCARD/ : {
        *(*)
    }
    */
}
