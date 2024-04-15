align 2
ud_goto dw 0

; Output:
; AX: CPU model
;     0 = 8086/8088
;     1 = 80186/80188
;     2 = 80286
;     3 = 80386
;     4 = 80486
;     5 = 80586
;     6 = 80686
;     7 = x86_64
;
; Local variables:
; - [bp - 4] = ud_handler_segment
; - [bp - 2] = ud_handler_offset
detect_cpu_model:
    push bp
    mov bp, sp

    sub sp, 4
    
    mov ax, str_detectCPU
    push ax
    call puts
    add sp, 2

    ; Save #UD handler and replace it with ours
    push ds

    xor ax, ax
    mov ds, ax
    
    mov ax, [0x0018]
    mov [ss:bp - 4], ax
    mov ax, [0x001a]
    mov [ss:bp - 2], ax

    mov ax, cs
    mov [0x001a], ax
    mov ax, ud_handler
    mov [0x0018], ax

    pop ds

    call find_cpu_model
    push ax

    ; Restore #UD handler
    push ds

    xor ax, ax
    mov ds, ax

    mov ax, [ss:bp - 4]
    mov [0x0018], ax
    mov ax, [ss:bp - 2]
    mov [0x001a], ax

    pop ds
    pop ax

    ; Function epilogue
    add sp, 4
    pop bp
    ret

find_cpu_model:
    push bp
    mov bp, sp

    ; If push immediate works, then the CPU is at least a 80186.
    cpu 186
    mov word [ud_goto], .detected_8086
    push word 0
    add sp, 2

    ; If sgdt works, then the CPU is at least a 80286.
    cpu 286
    mov word [ud_goto], .detected_80186
    push bx
    sub sp, 6
    mov bx, sp
    sgdt [bx]
    add sp, 6
    pop bx

    ; If a 32-bit instruction works, then the CPU is at least a 80386.
    cpu 386
    mov word [ud_goto], .detected_80286
    mov eax, 0

    ; If bswap works, then the CPU is at least a 80486.
    cpu 486
    mov word [ud_goto], .detected_80386
    bswap eax

    ; If rdstc works, then the CPU is at least a 80586.
    cpu 586
    mov word [ud_goto], .detected_80486
    rdtsc

    ; If CMOV works, then the CPU is at least a 80686.
    cpu 686
    mov word [ud_goto], .detected_80586
    cmovz ax, ax

    ; Check if the CPU supports long mode.
    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)
    jz .detected_80686

cpu 8086
.detected_x86_64:
    mov ax, 7
    jmp .epilogue

.detected_8086:
    xor ax, ax
    jmp .epilogue

.detected_80186:
    mov ax, 1
    add sp, 6
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

.epilogue:
    pop bp
    ret

; Stack:
; [sp] = ip
; [sp + 2] = cs
; [sp + 4] = flags
ud_handler:
    push ax
    push bx
    mov bx, sp
    mov ax, [cs:ud_goto]
    mov [bx + 4], ax
    pop bx
    pop ax
    iret

check_cpu:
    push bp
    mov bp, sp

    push bx

    call detect_cpu_model

    push ax

    mov ax, str_detectedCPU
    push ax
    call puts
    add sp, 2

    pop ax
    push ax

    shl ax, 1
    mov bx, cpu_model_table
    add bx, ax
    mov ax, [bx]
    push ax
    call puts
    add sp, 2

    mov ax, str_crlf
    push ax
    call puts
    add sp, 2

    pop ax

    cmp ax, 7
    jnz .unsupported_cpu

    pop bx
    pop bp
    ret

.unsupported_cpu:
    mov ax, str_unsupported_cpu
    push ax
    call puts
    add sp, 2
    jmp halt

cpu_model_table:
    dw str_cpu_8086
    dw str_cpu_80186
    dw str_cpu_80286
    dw str_cpu_80386
    dw str_cpu_80486
    dw str_cpu_80586
    dw str_cpu_80686
    dw str_cpu_x86_64
