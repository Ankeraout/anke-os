ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    .bootstrap16 (0x00000000) : {
        *(.bootstrap16)
    }

    . += 0x10000;

    .text : AT(ADDR(.text) - 0x10000) {
        *(.text)
        *(.text.*)
    }

    .data : AT(ADDR(.data) - 0x10000) {
        *(.data)
        *(.data.*)
    }

    .rodata : AT(ADDR(.rodata) - 0x10000) {
        *(.rodata)
        *(.rodata.*)
    }

    .bss (NOLOAD) : AT(ADDR(.bss) - 0x10000) {
        *(.bss)
        *(.bss.*)

        . = ALIGN(0x8000);
        g_isadmaBuffer = .;
        . += 0x8000;
        
        *(COMMON)
    }
}
