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

    .text : {
        *(.text)
        *(.text.*)
    } : text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } : rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data)
        *(.data.*)
    } : data

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .bss : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    } : data

    g_kernelEnd = .;

    /*
    /DISCARD/ : {
        *(*)
    }
    */
}
