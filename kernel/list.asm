struc ts_listElement
    .m_nextOffset: resw 1
    .m_nextSegment: resw 1
    .m_dataOffset: resw 1
    .m_dataSegment: resw 1
endstruc

section .text

; int list_remove(void **p_list, void *p_element)
list_remove:
    %define p_listOffset (bp + 4)
    %define p_listSegment (bp + 6)
    %define p_elementOffset (bp + 8)
    %define p_elementSegment (bp + 10)
    %define l_previousElementSegment (bp - 2)
    %define l_previousElementOffset (bp - 4)
    
    push bp
    mov bp, sp
    push es
    push di

    .checkListEmpty:
        ; if(*p_list == NULL) { return 0; }
        les di, [p_listOffset]
        mov ax, es:[di]
        or ax, es:[di + 2]
        jz .end

    .initPreviousElement:
        ; l_previousElement = NULL;
        xor ax, ax
        mov [l_previousElementSegment], ax
        mov [l_previousElementOffset], ax

    .getFirstElement:
        ; l_currentElement = *p_list;
        les di, es:[di]

    .checkElement:
        ; if(l_currentElement->m_data != p_element) { goto .getNextElement; }
        mov dx, es:[di + ts_listElement.m_dataSegment]
        cmp dx, [p_elementSegment]
        jnz .getNextElement

        mov ax, es:[di + ts_listElement.m_dataOffset]
        cmp ax, [p_elementOffset]
        jnz .getNextElement

    .removeElement:
        ; if(l_previousElement == NULL) { goto .removeFirstElement; }
        mov ax, [l_previousElementOffset]
        or ax, [l_previousElementSegment]
        jz .removeFirstElement

        ; l_previousElement->m_next = l_currentElement->m_next;
        mov dx, es:[di + ts_listElement.m_nextSegment]
        mov ax, es:[di + ts_listElement.m_nextOffset]
        les di, [l_previousElementOffset]
        mov es:[di + ts_listElement.m_nextSegment], dx
        mov es:[di + ts_listElement.m_nextOffset], ax

        ; Side effect: l_currentElement = l_previousElement;

    .getNextElement:
        ; l_previousElement = l_currentElement;
        mov [l_previousElementSegment], es
        mov [l_previousElementOffset], di

        ; l_currentElement = l_currentElement->m_next;
        les di, es:[di + ts_listElement.m_nextOffset]
        jmp .checkElement

    .removeFirstElement:
        ; *p_list = l_currentElement->m_next;
        mov dx, es:[di + ts_listElement.m_nextSegment]
        mov ax, es:[di + ts_listElement.m_nextOffset]
        les di, [p_listOffset]
        mov es:[di], ax
        mov es:[di + 2], dx
        
        ; Side effect: l_currentElement = l_currentElement->m_next;
        jmp .checkElement

    .end:
        pop di
        pop es
        pop bp
        ret

    %undef p_listOffset
    %undef p_listSegment
    %undef p_elementOffset
    %undef p_elementSegment
    %undef l_previousElementSegment
    %undef l_previousElementOffset
