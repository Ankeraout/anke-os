OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS {
    /**************************************************************************
     * Lower-half sections                                                    *
     **************************************************************************/
    . = 0x00100000;
    g_kernelStart = . + 0xc0000000;

    .multiboot : {
        *(.multiboot)
    }

    .text_low : ALIGN(0x1000) {
        *(.text_low)
    }

    .rodata_low : ALIGN(0x1000) {
        *(.rodata_low)
    }

    .data_low : ALIGN(0x1000) {
        *(.data_low)
    }

    .bss_low : ALIGN(0x1000) {
        *(.bss_low)
    }

    /**************************************************************************
     * Higher-half sections                                                   *
     **************************************************************************/
    . += 0xc0000000;

    .text ALIGN(0x1000) : AT(ADDR(.text) - 0xc0000000) {
        *(.text)
    }

    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - 0xc0000000) {
        *(.rodata)
    }

    .data ALIGN(0x1000) : AT(ADDR(.data) - 0xc0000000) {
        *(.data)
    }

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xc0000000) {
        *(.bss)
    }

    g_kernelEnd = .;

    /**************************************************************************
     * Symbol definitions                                                     *
     **************************************************************************/
    g_kernelPageDirectory = g_kernelPageDirectoryLow + 0xc0000000;
    g_kernelSelfMapPageTable = g_kernelSelfMapPageTableLow + 0xc0000000;
    g_kernelMemoryMap = g_kernelMemoryMapLow + 0xc0000000;
    g_kernelMemoryMapSize = g_kernelMemoryMapSizeLow + 0xc0000000;
    g_kernelPageTables = 0xffc00000;
}
