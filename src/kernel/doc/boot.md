# 1. Expected memory layout
When `main()` is called, the following conditions must be met:
- All memory entries described in the memory map are identity-mapped.
- Kernel memory is mapped to `0xffffffff80000000`.
