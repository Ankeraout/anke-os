ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 0x00100000;

    .multiboot : {
        *(.multiboot)
    }

    .bootstrap : {
        *(.bootstrap)
    }

    . += 0xc0000000;

    __kernel_start = .;

    .text ALIGN(0x1000): AT(ADDR(.text) - 0xc0000000) {
        *(.text)
    }

    .rodata ALIGN(0x1000): AT(ADDR(.rodata) - 0xc0000000) {
        *(.rodata)
    }
    
    .data ALIGN(0x1000): AT(ADDR(.data) - 0xc0000000) {
        *(.data)
    }
    
    .bss ALIGN(0x1000): AT(ADDR(.bss) - 0xc0000000) {
        *(COMMON)
        *(.bss)
    }

    __kernel_end = .;
}