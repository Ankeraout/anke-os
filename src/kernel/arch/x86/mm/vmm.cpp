#include <stdbool.h>
#include <stddef.h>

#include "kernel/arch/x86/paging.hpp"
#include "kernel/arch/x86/mm/mm.hpp"
#include "kernel/arch/x86/mm/pmm.hpp"
#include "kernel/libk/string.hpp"

namespace kernel {
    namespace arch {
        namespace x86 {
            namespace mm {
                namespace vmm {
                    enum {
                        FLAG_ALLOCATED = 1 << 0,
                        FLAG_SYSTEM = 1 << 1
                    };

                    static pageDirectoryEntry_t *pageDirectory = (pageDirectoryEntry_t *)(((size_t)&kernel_pageDirectory) + 0xc0000000);

                    void init() {
                        // Transform page directory PSE entries to page table entries
                        for(int i = 0; i < 1024; i++) {
                            if(pageDirectory[i].fields.present) {
                                if(pageDirectory[i].fields.pageSize) {
                                    pageTableEntry_t *newPageTableP = (pageTableEntry_t *)pmm::alloc(1);

                                    if(newPageTableP == NULL) {
                                        // TODO: die
                                    }

                                    pageTableEntry_t *newPageTableV = (pageTableEntry_t *)mm::mapTemporary(newPageTableP, mm::SERVICE_VMM);

                                    size_t address = pageDirectory[i].fields.pageTableAddress;
                                    
                                    pageDirectoryEntry_t pageDirectoryEntry = vmm::pageDirectory[i];

                                    for(int j = 0; j < 1024; j++) {
                                        newPageTableV[j].value = 0;
                                        newPageTableV[j].fields.writeThrough = 1;
                                        newPageTableV[j].fields.writePermission = 1;
                                        newPageTableV[j].fields.present = 1;
                                        newPageTableV[j].fields.pageAddress = address + j;
                                    }

                                    pageDirectoryEntry.fields.pageTableAddress = ((uint32_t)newPageTableP) >> 12;
                                    pageDirectoryEntry.fields.pageSize = 0;
                                    vmm::pageDirectory[i] = pageDirectoryEntry;
                                }
                            }
                        }
                    }

                    static void *map_search(size_t n, bool high) {
                        size_t pageDirectoryIndex = high ? 768 : 0;
                        int searchLength = high ? 256 : 768;
                        size_t pageTableIndex = 0;

                        size_t consecutiveFreePageCount = 0;
                        size_t firstFreePage_pageDirectoryIndex = pageDirectoryIndex;
                        size_t firstFreePage_pageTableIndex = 0;

                        for(int i = 0; i < searchLength; i++) {
                            if(vmm::pageDirectory[pageDirectoryIndex].fields.present) {
                                pageTableEntry_t *pageTable = (pageTableEntry_t *)mm::mapTemporary((const void *)(vmm::pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);

                                for(int j = 0; j < 1024; j++) {
                                    if(pageTable[j].fields.present || (pageTable[j].fields.available & (vmm::FLAG_ALLOCATED | vmm::FLAG_SYSTEM))) {
                                        consecutiveFreePageCount = 0;
                                    } else {
                                        if(consecutiveFreePageCount == 0) {
                                            firstFreePage_pageDirectoryIndex = pageDirectoryIndex;
                                            firstFreePage_pageTableIndex = pageTableIndex;
                                        }

                                        consecutiveFreePageCount++;
                                    }

                                    if(consecutiveFreePageCount >= n) {
                                        break;
                                    } else {
                                        pageTableIndex++;

                                        if(pageTableIndex == 1024) {
                                            pageDirectoryIndex++;
                                            pageTableIndex = 0;
                                        }
                                    }
                                }
                            } else {
                                consecutiveFreePageCount += 1024;
                            }

                            if(consecutiveFreePageCount >= n) {
                                return (void *)((firstFreePage_pageDirectoryIndex << 22) | (firstFreePage_pageTableIndex << 12));
                            }
                        }

                        return NULL;
                    }

                    static int createPageTable(size_t pageDirectoryIndex) {
                        void *pageTableP = pmm::alloc(1);

                        if(!pageTableP) {
                            return 1;
                        }

                        void *pageTableV = mm::mapTemporary(pageTableP, mm::SERVICE_VMM);

                        kernel::libk::memset(pageTableV, 0, 4096);

                        pageDirectory[pageDirectoryIndex].value = 0;
                        pageDirectory[pageDirectoryIndex].fields.pageTableAddress = ((uint32_t)pageTableP) >> 12;
                        pageDirectory[pageDirectoryIndex].fields.writeThrough = 1;
                        pageDirectory[pageDirectoryIndex].fields.writePermission = 1;
                        pageDirectory[pageDirectoryIndex].fields.present = 1;

                        return 0;
                    }

                    void *map2(const void *paddr, void *vaddr, size_t n) {
                        size_t pageNumber = ((size_t)paddr) >> 12;
                        size_t address = (size_t)vaddr;
                        size_t pageDirectoryIndex = address >> 22;
                        size_t pageTableIndex = (address >> 12) & 0x3ff;

                        if(!pageDirectory[pageDirectoryIndex].fields.present) {
                            if(createPageTable(pageDirectoryIndex)) {
                                return NULL;
                            }
                        }
                        
                        pageTableEntry_t *pageTable = (pageTableEntry_t *)mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);

                        for(size_t i = 0; i < n; i++) {
                            pageTable[pageTableIndex].fields.pageAddress = pageNumber;
                            pageTable[pageTableIndex].fields.global = 0;
                            pageTable[pageTableIndex].fields.zero = 0;
                            pageTable[pageTableIndex].fields.dirty = 0;
                            pageTable[pageTableIndex].fields.accessed = 0;
                            pageTable[pageTableIndex].fields.disableCache = 0;
                            pageTable[pageTableIndex].fields.writeThrough = 1;
                            pageTable[pageTableIndex].fields.writePermission = 1;
                            pageTable[pageTableIndex].fields.present = 1;

                            pageTableIndex++;
                            pageNumber++;

                            if(pageTableIndex == 1024) {
                                pageTableIndex = 0;
                                pageDirectoryIndex++;

                                if(!pageDirectory[pageDirectoryIndex].fields.present) {
                                    createPageTable(pageDirectoryIndex);
                                } else {
                                    mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);
                                }
                            }
                        }

                        return vaddr;
                    }

                    void *map(const void *paddr, size_t n, bool high) {
                        void *mapAddress = map_search(n, high);

                        if(!mapAddress) {
                            return NULL;
                        }

                        return map2(paddr, mapAddress, n);
                    }

                    int unmap(const void *vaddr, size_t n) {
                        size_t address = (size_t)vaddr;
                        size_t pageDirectoryIndex = address >> 22;
                        size_t pageTableIndex = (address >> 12) & 0x3ff;

                        pageTableEntry_t *pageTable = NULL;

                        if(pageDirectory[pageDirectoryIndex].fields.present) {
                            pageTable = (pageTableEntry_t *)mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);
                        }

                        for(size_t i = 0; i < n; i++) {
                            if(pageTable) {
                                pageTable[pageTableIndex].fields.present = 0;
                            }

                            pageTableIndex++;

                            if(pageTableIndex == 1024) {
                                pageTableIndex = 0;
                                pageDirectoryIndex++;

                                if(pageDirectory[pageDirectoryIndex].fields.present) {
                                    pageTable = (pageTableEntry_t *)mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);
                                } else {
                                    pageTable = NULL;
                                }
                            }
                        }

                        return 0;
                    }

                    int unmap2(const void *vaddr, size_t n) {
                        size_t address = (size_t)vaddr;
                        size_t pageDirectoryIndex = address >> 22;
                        size_t pageTableIndex = (address >> 12) & 0x3ff;

                        pageTableEntry_t *pageTable = NULL;

                        if(pageDirectory[pageDirectoryIndex].fields.present) {
                            pageTable = (pageTableEntry_t *)mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);
                        }

                        for(size_t i = 0; i < n; i++) {
                            if(pageTable) {
                                pmm::free((const void *)(pageTable[pageTableIndex].fields.pageAddress << 12), 1);
                                pageTable[pageTableIndex].fields.present = 0;
                            }

                            pageTableIndex++;

                            if(pageTableIndex == 1024) {
                                pageTableIndex = 0;
                                pageDirectoryIndex++;

                                if(pageDirectory[pageDirectoryIndex].fields.present) {
                                    pageTable = (pageTableEntry_t *)mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);
                                } else {
                                    pageTable = NULL;
                                }
                            }
                        }

                        return 0;
                    }

                    void *alloc(size_t n, bool high) {
                        void *vaddr = map_search(n, high);

                        if(vaddr == NULL) {
                            return NULL;
                        }

                        size_t address = (size_t)vaddr;
                        size_t pageDirectoryIndex = address >> 22;
                        size_t pageTableIndex = (address >> 12) & 0x3ff;

                        if(!pageDirectory[pageDirectoryIndex].fields.present) {
                            if(createPageTable(pageDirectoryIndex)) {
                                return NULL;
                            }
                        }
                        
                        pageTableEntry_t *pageTable = (pageTableEntry_t *)mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);

                        for(size_t i = 0; i < n; i++) {
                            pageTableEntry_t pageTableEntry;

                            pageTableEntry.value = 0;
                            pageTableEntry.fields.available = FLAG_ALLOCATED;

                            pageTable[pageTableIndex] = pageTableEntry;

                            pageTableIndex++;

                            if(pageTableIndex == 1024) {
                                pageTableIndex = 0;
                                pageDirectoryIndex++;

                                if(!pageDirectory[pageDirectoryIndex].fields.present) {
                                    createPageTable(pageDirectoryIndex);
                                } else {
                                    mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);
                                }
                            }
                        }

                        return vaddr;
                    }

                    int free(const void *vaddr, size_t n) {
                        size_t address = (size_t)vaddr;
                        size_t pageDirectoryIndex = address >> 22;
                        size_t pageTableIndex = (address >> 12) & 0x3ff;

                        while(!pageDirectory[pageDirectoryIndex].fields.present) {
                            if(n > 1024) {
                                n -= 1024;
                                pageDirectoryIndex++;
                            } else {
                                return 0;
                            }
                        }
                        
                        pageTableEntry_t *pageTable = (pageTableEntry_t *)mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);

                        for(size_t i = 0; i < n; i++) {
                            pageTableEntry_t pageTableEntry;

                            pageTableEntry.value = 0;
                            pageTableEntry.fields.available &= ~FLAG_ALLOCATED;

                            pageTable[pageTableIndex] = pageTableEntry;

                            pageTableIndex++;

                            if(pageTableIndex == 1024) {
                                pageTableIndex = 0;
                                pageDirectoryIndex++;

                                while(!pageDirectory[pageDirectoryIndex].fields.present) {
                                    if(n > 1024) {
                                        n -= 1024;
                                        pageDirectoryIndex++;
                                    } else {
                                        return 0;
                                    }
                                }

                                mm::mapTemporary((const void *)(pageDirectory[pageDirectoryIndex].fields.pageTableAddress << 12), mm::SERVICE_VMM);
                            }
                        }

                        return 0;
                    }
                }
            }
        }
    }
}
