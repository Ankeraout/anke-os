# AnkeOS kernel memory map
The kernel only uses 4-level paging (48-bit linear address space).
## Kernel context
The kernel context is loaded when the kernel is doing kernel-specific stuff that
is not related to a user task.
Address | Description
-|-
0x0000000000000000-0x00007fffffffffff | Unused
0x0000800000000000-0xffff7fffffffffff | Invalid(Non-canonical addresses)
0xffff800000000000-0xffff8fffffffffff | Direct memory map
0xffff900000000000-0xffff9fffffffffff | MMIO
0xffffa00000000000-0xffffff7fffffffff | Unused
0xffffff8000000000-0xffffffff7fffffff | Kernel heap (510 GiB)
0xffffffff80000000-0xffffffffffffffff | Kernel image (2 GiB)
## User context
The user context is loaded when a user task is running.
Address | Description
-|-
0x0000000000000000-0x00007fffffffffff | User memory space
0x0000800000000000-0xffff7fffffffffff | Invalid(Non-canonical addresses)
0xffff800000000000-0xffffff7fffffffff | Unused
0xffffff8000000000-0xffffffff7fffffff | Kernel heap (510 GiB)
0xffffffff80000000-0xffffffffffffffff | Kernel image (2 GiB)
## Global mappings
The following mappings are global among all memory contexts.
Address | Description
-|-
0x0000800000000000-0xffff7fffffffffff | Invalid(Non-canonical addresses)
0xffff800000000000-0xffffff7fffffffff | Unused
0xffffff8000000000-0xffffffff7fffffff | Kernel heap (510 GiB)
0xffffffff80000000-0xffffffffffffffff | Kernel image (2 GiB)

The kernel heap and image are always loaded at all times, so that when an
interrupt occurs during the execution of a user task, the processor can jump to
kernel code.
If the user code performs a system call, the kernel will execute in the context
of the user task (to be able to manipulate the heap of the user task). If the
system call requires interacting with the hardware, the kernel must switch to
the kernel memory context to access MMIO.
## Bootstrap
At bootstrap, the bootloader is expected to give the kernel a pointer to the
higher-half direct map (HHDM). This section maps the entire usable memory.
The kernel is expected to be mapped at 0xffffffff80000000.
