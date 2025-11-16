# AnkeOS kernel memory map
The kernel only uses 4-level paging (48-bit linear address space).
Address | Description
-|-
0x0000000000000000-0x00007fffffffffff | User memory space
0xffff800000000000-0xffff8fffffffffff | Direct memory map
0xffff900000000000-0xffff9fffffffffff | MMIO
0xffffa00000000000-0xffffffff7fffffff | Unused
0xffffffff80000000-0xffffffffffffffff | Kernel memory space
## Bootstrap
At bootstrap, the bootloader is expected to give the kernel a pointer to the
higher-half direct map (HHDM). This section maps the entire usable memory.
The kernel is expected to be mapped at 0xffffffff80000000.
