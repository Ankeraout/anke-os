hw_detect_cpu:
%define l_cpuModel bp - 2
    push bp
    mov bp, sp

    sub sp, 2

    mov ax, g_str_hw_detect_cpu
    push ax
    call puts
    add sp, 2

    call hw_cpu_get_model
    
    mov [l_cpuModel], ax

    push si
    mov si, ax

    shl si, 1
    add si, g_str_hw_detect_cpu_table
    mov ax, [si]
    push ax
    call puts
    add sp, 2

    mov ax, [l_cpuModel]

    pop si

    add sp, 2

    pop bp
    ret
%undef l_cpuModel

hw_cpu_get_model:
%define l_udSegment bp - 2
%define l_udOffset bp - 4
    push bp
    mov bp, sp

    sub sp, 4

    pushf
    push es

    cli

    xor ax, ax
    mov es, ax

    mov ax, [es:0x0018]
    mov [l_udOffset], ax
    mov ax, [es:0x001a]
    mov [l_udSegment], ax

    mov ax, hw_cpu_get_model_ud
    mov [es:0x0018], ax
    mov ax, cs
    mov [es:0x001a], ax

    pop es

    cpu 186
    mov word [g_hw_detect_cpu_goto], .detected_8086
    push word 0
    add sp, 2

    cpu 286
    mov word [g_hw_detect_cpu_goto], .detected_80186
    push bx
    mov bx, sp
    sgdt [bx - 6]
    pop bx

    cpu 386
    mov word [g_hw_detect_cpu_goto], .detected_80286
    mov eax, 0

    cpu 486
    mov word [g_hw_detect_cpu_goto], .detected_80386
    bswap eax

    cpu 586
    mov word [g_hw_detect_cpu_goto], .detected_80486
    rdtsc

    cpu 686
    mov word [g_hw_detect_cpu_goto], .detected_80586
    cmovz ax, ax

    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)
    jz .detected_80686

cpu 8086

.detected_x64:
    mov ax, 7

.epilogue:
    push ax
    push es
    xor ax, ax
    mov es, ax

    mov ax, [l_udOffset]
    mov [es:0x0018], ax
    mov ax, [l_udSegment]
    mov [es:0x001a], ax

    pop es
    pop ax
    popf

    add sp, 4

    pop bp
    ret

.detected_8086:
    xor ax, ax
    jmp .epilogue

.detected_80186:
    mov ax, 1
    pop bx
    jmp .epilogue

.detected_80286:
    mov ax, 2
    jmp .epilogue

.detected_80386:
    mov ax, 3
    jmp .epilogue

.detected_80486:
    mov ax, 4
    jmp .epilogue

.detected_80586:
    mov ax, 5
    jmp .epilogue

.detected_80686:
    mov ax, 6
    jmp .epilogue

hw_cpu_get_model_ud:
    push ax
    push bx
    mov bx, sp
    mov ax, [cs:g_hw_detect_cpu_goto]
    mov [bx + 4], ax
    pop bx
    pop ax
    iret

%undef l_udSegment
%undef l_udOffset

g_hw_detect_cpu_goto dw 0
