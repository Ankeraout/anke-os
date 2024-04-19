# bootstrap32
bootstrap32's role is to prepare the 64-bit environment for bootstrap64.
For this, the following steps are required:
- Load a GDT with 64-bit segment selectors
- Initialize 64-bit paging structures (PML4, PDPT, PD and PT).
Note that bootstrap32 will only identity map the first 2 MiB of memory.
- Enable PAE
- Load CR3 with the physical address of the PML4
- Enable long mode
- Enable paging
- Reload segment registers
- Jump to 64-bit code

## Memory map
bootstrap32 uses 16 KiB of RAM from **0xc000** to **0xffff** to store paging
structures. It does not modify or overwrite existing in-memory structures that
appear in bootstrap16's memory map, which makes them also available for
bootstrap64.