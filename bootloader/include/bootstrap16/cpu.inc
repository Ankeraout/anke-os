; int cpu_check(void)
;
; Summary:
; Checks if the CPU supports x86_64 mode.
;
; Returns:
; An integer that indicates the detected CPU status:
; - 0: The CPU supports x86_64.
; - 1: The CPU does not support x86_64.
; - 2: The CPU does not support the CPUID opcode.
;
; Local variables:
; - [bp - 2]: int l_returnValue
; - [bp - 4]: uint16_t l_oldHandlerSegment
; - [bp - 6]: uint16_t l_oldHandlerOffset
; - [bp - 8]: uint16_t l_previousES
cpu_check:
    push bp
    mov bp, sp
    sub sp, 6

    ; Save ES
    push es

    ; Save #UD handler and replace it with ours
    xor ax, ax
    mov es, ax
    
    mov ax, [es:0x0018]
    mov [bp - 6], ax
    mov ax, [es:0x001a]
    mov [bp - 4], ax

    mov ax, cs
    mov [es:0x001a], ax
    mov ax, .ud_handler
    mov [es:0x0018], ax

    ; Check if the CPU supports x86_64
    cpu 586

    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)
    
    cpu 8086
    
    jz .unsupported_cpu

.restore_ud_handler:
    ; Restore #UD handler
    xor ax, ax
    mov es, ax

    mov ax, [bp - 6]
    mov [es:0x0018], ax
    mov ax, [bp - 4]
    mov [es:0x001a], ax

    ; Restore ES
    pop es

    ; Load the return value into AX
    mov ax, [bp - 2]

    ; Function epilogue
    add sp, 6
    pop bp
    ret

.ud_handler:
    mov word [bp - 2], 2                ; Set the return value to 2.
    pop ax                              ; Remove IP from the stack.
    mov ax, .restore_ud_handler         
    push ax                             ; Continue after the error.
    iret                                ; Return from interrupt.

.unsupported_cpu:
    mov word [bp - 2], 1                ; Set the return value to 1.
    jmp .restore_ud_handler             ; Continue execution.
