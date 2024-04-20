str_error_cpu:
    db "Fatal error: your CPU does not support 64-bit mode.", 13, 10, 0

str_error_a20:
    db "Fatal error: failed to enable the A20 gate.", 13, 10, 0

str_error_mmap:
    db "Fatal error: failed to initialize the memory map.", 13, 10, 0

str_error_sysinfo:
    db "Fatal error: failed to initialize the system information structure."
    db 13, 10, 0
    
str_bootloader_version:
    db "AnkeOS bootloader 0.1.0", 0
