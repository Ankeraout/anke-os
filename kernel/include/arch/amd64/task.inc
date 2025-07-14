%ifndef __INCLUDE_ARCH_AMD64_TASK_INC__
%define __INCLUDE_ARCH_AMD64_TASK_INC__

struc ts_task_context
    .m_rax: resq 1
    .m_rbx: resq 1
    .m_rcx: resq 1
    .m_rdx: resq 1
    .m_rbp: resq 1
    .m_rsp: resq 1
    .m_rsi: resq 1
    .m_rdi: resq 1
    .m_r8: resq 1
    .m_r9: resq 1
    .m_r10: resq 1
    .m_r11: resq 1
    .m_r12: resq 1
    .m_r13: resq 1
    .m_r14: resq 1
    .m_r15: resq 1
    .m_rip: resq 1
    .m_cs: resq 1
    .m_ds: resq 1
    .m_es: resq 1
    .m_fs: resq 1
    .m_gs: resq 1
    .m_ss: resq 1
    .m_rflags: resq 1
endstruc

%endif
