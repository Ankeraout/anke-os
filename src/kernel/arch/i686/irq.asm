section .bss
global irq_handlers
irq_handlers:
    resd 16

global irq_handlers_args
irq_handlers_args:
    resd 16
    