#pragma once

#include <stdint.h>

namespace kernel {
    namespace arch {
        namespace x86 {
            namespace mmap {
                typedef struct {
                    union {
                        uint64_t base;

                        struct {
                            uint32_t base_low;
                            uint32_t base_high;
                        } __attribute__((packed));
                    };

                    union {
                        uint64_t length;

                        struct {
                            uint32_t length_low;
                            uint32_t length_high;
                        } __attribute__((packed));
                    };

                    uint32_t type;
                    uint32_t padding;
                } __attribute__((packed)) entry_t;

                void init();
                const entry_t *get();
                int getLength();
            }
        }
    }
}
