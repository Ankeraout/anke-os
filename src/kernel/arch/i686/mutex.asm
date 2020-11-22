bits 32

section .text

global mutex_acquire
;mutex_t *mutex
%define arg_mutex (ebp + 8)
mutex_acquire:
    push ebp
    mov ebp, esp

.loop:
    mov eax, 1
    xchg eax, [arg_mutex]
    test eax, eax
    jnz .loop

.acquired:
    pop ebp
    ret
    
%undef arg_mutex

global mutex_tryAcquire
;mutex_t *mutex
%define arg_mutex (ebp + 8)
mutex_tryAcquire:
    push ebp
    mov ebp, esp

    mov eax, 1
    xchg eax, [arg_mutex]

    pop ebp
    ret
    
%undef arg_mutex

global mutex_release
;mutex_t *mutex
%define arg_mutex (esp + 4) ;mutex_t *mutex
mutex_release:
    mov eax, [esp + 4]
    mov dword [eax], 0
    ret

%undef arg_mutex
