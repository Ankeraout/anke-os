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

    .text : {
        *(.text)
    } : text

    . += CONSTANT(MAXPAGESIZE);

    .rodata : {
        *(.rodata)
    } : rodata

    . += CONSTANT(MAXPAGESIZE);

    .data : {
        *(.data)
    } : data

    .bss : {
        *(COMMON)
        *(.bss)
    } : data

    /DISCARD/ : {
        *(*)
    }
}
