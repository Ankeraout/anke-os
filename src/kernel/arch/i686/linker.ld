ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 0x00100000;

    .multiboot.text : {
        *(.multiboot.text)
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

    .init ALIGN(0x1000): AT(ADDR(.init) - 0xc0000000) {
        *(.init)
    }

    .fini ALIGN(0x1000): AT(ADDR(.fini) - 0xc0000000) {
        *(.fini)
    }

    __kernel_end = .;
}
