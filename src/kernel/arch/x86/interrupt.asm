section .data
global interrupt_handlers
interrupt_handlers:
    times 16 dd 0

global interrupt_handlers_args
interrupt_handlers_args:
    times 16 dd 0
