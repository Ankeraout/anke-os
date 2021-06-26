#pragma once

namespace kernel {
    namespace arch {
        namespace x86 {
            namespace mm {
                namespace pmm {
                    void init();
                    void *alloc(int n);
                    void free(const void *pages, int n);
                }
            }
        }
    }
}
