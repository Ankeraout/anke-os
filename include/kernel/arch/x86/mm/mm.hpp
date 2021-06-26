#pragma once

#include <stddef.h>

namespace kernel {
    namespace arch {
        namespace x86 {
            namespace mm {
                namespace mm {
                    typedef enum {
                        SERVICE_PMM,
                        SERVICE_VMM
                    } service_t;

                    void init();
                    void *mapTemporary(const void *pageAddress, service_t service);
                    void unmapTemporary(service_t service);
                    void *alloc(size_t n);
                    void free(void *buffer, size_t n);
                }
            }
        }
    }
}
