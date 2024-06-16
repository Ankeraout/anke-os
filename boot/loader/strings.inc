g_str_announce db "AnkeOS bootloader", 13, 10, 0
g_str_hw_detect db "+ Detecting hardware...", 13, 10, 0
g_str_hw_detect_cpu db "|-- CPU: ", 0
g_str_hw_detect_cpu_8086 db "8086/8088", 13, 10, 0
g_str_hw_detect_cpu_80186 db "80186/80188", 13, 10, 0
g_str_hw_detect_cpu_80286 db "80286", 13, 10, 0
g_str_hw_detect_cpu_80386 db "80386", 13, 10, 0
g_str_hw_detect_cpu_80486 db "80486", 13, 10, 0
g_str_hw_detect_cpu_80586 db "80586", 13, 10, 0
g_str_hw_detect_cpu_80686 db "80686", 13, 10, 0
g_str_hw_detect_cpu_x64 db "x86_64", 13, 10, 0
g_str_hw_detect_cpu_table dw \
    g_str_hw_detect_cpu_8086, \
    g_str_hw_detect_cpu_80186, \
    g_str_hw_detect_cpu_80286, \
    g_str_hw_detect_cpu_80386, \
    g_str_hw_detect_cpu_80486, \
    g_str_hw_detect_cpu_80586, \
    g_str_hw_detect_cpu_80686, \
    g_str_hw_detect_cpu_x64
g_str_hw_detect_pci db "|-- PCI: ", 0
g_str_hw_detect_pci_no db "No PCI bus detected.", 13, 10, 0
g_str_hw_detect_pci_version db "PCI version ", 0
g_str_hw_detect_pci_version_end db 13, 10, 0
g_str_unsupported_cpu db "Your CPU is too old to run AnkeOS.", 13, 10, 0
g_str_init_gdt db "Initializing GDT...", 13, 10, 0
g_str_init_pmode db "Setting 32-bit protected mode...", 13, 10, 0
