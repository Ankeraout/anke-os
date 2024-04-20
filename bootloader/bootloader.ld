OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS {
    text16  PT_LOAD FLAGS(5);
    text    PT_LOAD FLAGS(5);
    rodata  PT_LOAD FLAGS(4);
    data    PT_LOAD FLAGS(6);
}

SECTIONS {
    . = 0x00000000;

    .bootstrap16 : {
        *(.bootstrap16)
    } : text16
    
    . += 0x00010000;

    .bootstrap32 : AT(ADDR(.bootstrap32) - 0x00010000) {
        *(.bootstrap32)
    } : text

    .text : AT (ADDR(.text) - 0x00010000) {
        *(.text)
        *(.text.*)
    } : text

    .data : AT (ADDR(.data) - 0x00010000) {
        *(.data)
    } : data

    .rodata : AT (ADDR(.rodata) - 0x00010000) {
        *(.rodata)
        *(.rodata.*)
    } : rodata

    .bss : AT (ADDR(.bss) - 0x00010000) {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    } : data

    /DISCARD/ : {
        *(*)
    }
}
