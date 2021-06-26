#include <stdbool.h>
#include <stddef.h>

namespace kernel {
    namespace arch {
        namespace x86 {
            namespace mm {
                namespace vmm {
                    // Initializes virtual memory manager
                    void init();

                    // Maps n pages at physical address paddr to virtual address space
                    void *map(const void *paddr, size_t n, bool high);

                    // Maps n pages at physical address paddr to virtual address vaddr
                    void *map2(const void *paddr, void *vaddr, size_t n);

                    // Unmaps n pages at virtual address vaddr but don't release the physical pages
                    // to physical memory manager.
                    int unmap(const void *vaddr, size_t n);

                    // Unmaps n pages at virtual address vaddr and release the physical pages to
                    // physical memory manager.
                    int unmap2(const void *vaddr, size_t n);

                    // Allocates n pages in virtual memory space.
                    void *alloc(size_t n, bool high);

                    // Frees n pages in virtual memory space. If the pages were mapped, they are
                    // unmapped but not released to physical memory manager.
                    int free(const void *vaddr, size_t n);
                }
            }
        }
    }
}
