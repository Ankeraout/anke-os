OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

PHDRS {
    code16  PT_LOAD FLAGS(5);
    text    PT_LOAD FLAGS(5);
    rodata  PT_LOAD FLAGS(4);
    data    PT_LOAD FLAGS(6);
}

SECTIONS {
    . = 0x00008000;

    .bootstrap16 : {
        *(.bootstrap16)
    } : code16

    .realmode : {
        *(.realmode)
    } : code16

    .text : {
        *(.text)
        *(.text.*)
    } : text

    .data : {
        *(.data)
        *(.data.*)
    } : data

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } : rodata

    .bss : {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    } : data

    /DISCARD/ : {
        *(*)
    }
}
