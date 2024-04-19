# bootstrap64
bootstrap64 is a very small program written in x86_64 assembly that initializes
the execution environment for executing C code. It performs the following
initializations steps:
- Initialize the stack pointer
- Call `main`, with the address of the system information structure as
the first parameter (in the `rdi` register)
