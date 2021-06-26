#include <stdint.h>

namespace kernel {
    namespace arch {
        namespace x86 {
            typedef union {
                uint32_t value;

                struct {
                    uint32_t present : 1;
                    uint32_t writePermission : 1;
                    uint32_t userPermission : 1;
                    uint32_t writeThrough : 1;
                    uint32_t disableCache : 1;
                    uint32_t accessed : 1;
                    uint32_t zero : 1;
                    uint32_t pageSize : 1;
                    uint32_t available : 4;
                    uint32_t pageTableAddress : 20;
                } __attribute__((packed)) fields;
            } pageDirectoryEntry_t;

            typedef union {
                uint32_t value;

                struct {
                    uint32_t present : 1;
                    uint32_t writePermission : 1;
                    uint32_t userPermission : 1;
                    uint32_t writeThrough : 1;
                    uint32_t disableCache : 1;
                    uint32_t accessed : 1;
                    uint32_t dirty : 1;
                    uint32_t zero : 1;
                    uint32_t global : 1;
                    uint32_t available : 3;
                    uint32_t pageAddress : 20;
                } __attribute__((packed)) fields;
            } pageTableEntry_t;

            extern "C" pageDirectoryEntry_t kernel_pageDirectory[1024];
        }
    }
}
