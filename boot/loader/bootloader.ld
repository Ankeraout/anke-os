ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 0x00010000;

    .bootstrap16 : AT(. - 0x00010000) {
        *(.bootstrap16)
    }

    .text : {
        *(.text)
        *(.text.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .bss (NOLOAD) : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
}
