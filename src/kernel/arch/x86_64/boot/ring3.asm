bits 64
section .text

global ring3_task
global ring3_task_end
ring3_task:
    jmp $
ring3_task_end:
