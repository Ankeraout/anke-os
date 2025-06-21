; struct ts_sequence {
;     const char *m_name;
;     int (*m_func)(void);
; }
struc ts_sequenceElement
    .m_name: resw 1
    .m_func: resw 1
endstruc

; struct ts_sequence {
;     const char *m_name;
;     size_t m_count;
;     const struct ts_sequenceElement m_elements[]
; }
struc ts_sequence
    .m_name: resw 1
    .m_count: resw 1
    .m_elements:
endstruc

section .text

; int sequence_run(const struct ts_sequence *p_sequence)
sequence_run:
    %define p_sequence (bp + 4)

    push bp
    mov bp, sp

    push bx
    push si

    mov ax, g_sequence_msg_runningSequence1
    push ds
    push ax
    call printk
    add sp, 4

    mov bx, [p_sequence]
    push ds
    push word [bx + ts_sequence.m_name]
    call printk
    add sp, 4

    mov ax, g_sequence_msg_runningSequence2
    push ds
    push ax
    call printk
    add sp, 4

    xor cx, cx

    .loop:
        xor ax, ax
        cmp cx, [bx + ts_sequence.m_count]
        jz .loop_end

        mov ax, ts_sequenceElement_size
        mul cx
        mov si, ax

        mov ax, g_sequence_msg_runningElement1
        push ds
        push ax
        call printk
        add sp, 4

        mov ax, [bx + si + ts_sequence.m_elements + ts_sequenceElement.m_name]
        push ds
        push ax
        call printk
        add sp, 4

        mov ax, g_sequence_msg_runningElement2
        push ds
        push ax
        call printk
        add sp, 4
        
        push cx
        call [bx + si + ts_sequence.m_elements + ts_sequenceElement.m_func]
        pop cx

        test ax, ax
        jnz .failed

        mov ax, g_sequence_msg_successElement
        push ds
        push ax
        call printk
        add sp, 4

        inc cx
        jmp .loop

    .failed:
        mov dx, g_sequence_msg_errorElement
        push ds
        push dx
        call printk
        add sp, 4

        jmp .loop_end

    .loop_end:

    mov ax, g_sequence_msg_endOfSequence
    push ds
    push ax
    call printk
    add sp, 4

    pop si
    pop bx

    pop bp
    ret

    %undef p_sequence

section .rodata
g_sequence_msg_runningSequence1: db "Running sequence ", '"', 0
g_sequence_msg_runningSequence2: db '"', "...", 13, 10, 0
g_sequence_msg_runningElement1: db "  - Running element ", '"', 0
g_sequence_msg_runningElement2: db '"', "... ", 0
g_sequence_msg_errorElement: db "Error!", 13, 10, 0
g_sequence_msg_successElement: db "Success!", 13, 10, 0
g_sequence_msg_endOfSequence: db "End of sequence.", 13, 10, 0
g_sequence_msg_crlf: db 13, 10, 0
