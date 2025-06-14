%define E_THREADSTATUS_READY 0
%define E_THREADSTATUS_RUNNING 1
%define E_THREADSTATUS_SUSPENDED 2
%define E_THREADSTATUS_WAITING 3

struc ts_thread
    .m_status: resw 1
    .m_taskSegment: resw 1
    .m_taskOffset: resw 1
    .m_previousThreadSegment: resw 1
    .m_previousThreadOffset: resw 1
    .m_nextThreadSegment: resw 1
    .m_nextThreadOffset: resw 1
endstruc
