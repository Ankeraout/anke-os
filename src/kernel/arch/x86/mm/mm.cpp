#include <stdbool.h>
#include <stddef.h>

#include "kernel/arch/x86/assembly.hpp"
#include "kernel/arch/x86/paging.hpp"
#include "kernel/arch/x86/mm/mm.hpp"
#include "kernel/arch/x86/mm/pmm.hpp"
#include "kernel/arch/x86/mm/vmm.hpp"

namespace kernel {
    namespace arch {
        namespace x86 {
            namespace mm {
                namespace mm {
                    __attribute__((aligned(0x1000))) pageTableEntry_t temporaryPageTable[1024];
                    void *temporaryPageTable_base;

                    void init() {
                        pageDirectoryEntry_t *pageDirectory = (pageDirectoryEntry_t *)(((size_t)&kernel_pageDirectory) + 0xc0000000);
                        pageDirectoryEntry_t newEntry = {
                            .fields = {
                                .present = 1,
                                .writePermission = 1,
                                .userPermission = 0,
                                .writeThrough = 1,
                                .disableCache = 0,
                                .accessed = 0,
                                .zero = 0,
                                .pageSize = 0,
                                .available = 0,
                                .pageTableAddress = (((size_t)temporaryPageTable) - 0xc0000000) >> 12
                            }
                        };

                        bool found = false;
                        
                        for(int i = 0; i < 1024 && !found; i++) {
                            if(!pageDirectory[i].fields.present) {
                                pageDirectory[i] = newEntry;
                                temporaryPageTable_base = (void *)(i << 22);
                                found = true;
                            }
                        }

                        if(!found) {
                            // TODO: die
                        }
                    }

                    void *mapTemporary(const void *pageAddress, service_t service) {
                        void *mappedPageAddress = (void *)(((size_t)temporaryPageTable_base) | (service << 12));

                        pageTableEntry_t newEntry = {
                            .fields = {
                                .present = 1,
                                .writePermission = 1,
                                .userPermission = 0,
                                .writeThrough = 1,
                                .disableCache = 0,
                                .accessed = 0,
                                .dirty = 0,
                                .zero = 0,
                                .global = 0,
                                .available = 0,
                                .pageAddress = ((size_t)pageAddress) >> 12
                            }
                        };

                        temporaryPageTable[service] = newEntry;

                        invlpg(mappedPageAddress);

                        return mappedPageAddress;
                    }

                    void unmapTemporary(service_t service) {
                        void *mappedPageAddress = (void *)(((size_t)temporaryPageTable_base) | (service << 12));

                        temporaryPageTable[service].fields.present = 0;

                        invlpg(mappedPageAddress);
                    }

                    void *alloc(size_t n) {
                        void *vaddr = vmm::alloc(n, true);

                        if(!vaddr) {
                            return NULL;
                        }

                        for(size_t i = 0; i < n; i++) {
                            void *vaddr2 = (void *)(((size_t)vaddr) + (i * 0x1000));
                            void *paddr = pmm::alloc(1);

                            if(!paddr) {
                                if(i) {
                                    vmm::unmap2(vaddr, i);
                                }

                                free(vaddr, n);

                                return NULL;
                            }

                            vmm::map2(paddr, vaddr2, 1);
                        }

                        return vaddr;
                    }

                    void free(void *buffer, size_t n) {
                        vmm::unmap2(buffer, n);
                        vmm::free(buffer, n);
                    }
                }
            }
        }
    }
}
