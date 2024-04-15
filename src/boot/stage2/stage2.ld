OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS {
    . = 0x00000000;

    .bootstrap16 : {
        *(.bootstrap16)
    }
    
    . += 0x00010000;

    .bootstrap32 : AT(ADDR(.bootstrap32) - 0x00010000) {
        *(.bootstrap32)
    }

    .text : AT (ADDR(.text) - 0x00010000) {
        *(.text)
        *(.text.*)
    }

    .data : AT (ADDR(.data) - 0x00010000) {
        *(.data)
    }

    .rodata : AT (ADDR(.rodata) - 0x00010000) {
        *(.rodata)
        *(.rodata.*)
    }

    .bss : AT (ADDR(.bss) - 0x00010000) {
        *(COMMON)
        *(.bss)
    }

    /DISCARD/ : {
        *(*)
    }
}
