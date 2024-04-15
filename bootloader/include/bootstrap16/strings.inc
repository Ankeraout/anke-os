str_announce db "AnkeOS bootloader 0.1.0", 13, 10, 0
str_bootstrap16 db "16-bit bootstrap procedure started.", 13, 10, 0
str_detectCPU db "Checking CPU capabilities...", 13, 10, 0
str_detectedCPU db "Detected CPU: ", 0
str_crlf db 13, 10, 0
str_cpu_8086 db "8086/8088", 0
str_cpu_80186 db "80186/80188", 0
str_cpu_80286 db "80286", 0
str_cpu_80386 db "80386", 0
str_cpu_80486 db "80486", 0
str_cpu_80586 db "80586", 0
str_cpu_80686 db "80686", 0
str_cpu_x86_64 db "x86_64", 0
str_unsupported_cpu db "Your CPU does not support running 64-bit code.", 13, 10, 0
str_a20_ok db "Enabled A20 gate successfully.", 13, 10, 0
str_a20_nok db "Failed to enable A20 gate.", 13, 10, 0
