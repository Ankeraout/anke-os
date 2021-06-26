#include <stdbool.h>
#include <stddef.h>

#include "kernel/arch/x86/mmap.hpp"
#include "kernel/arch/x86/mm/mm.hpp"
#include "kernel/libk/string.hpp"

namespace kernel {
    namespace arch {
        namespace x86 {
            namespace mm {
                namespace pmm {
                    static __attribute__((aligned(0x1000))) uint8_t memoryPageTable[32 * 4096];
                    static size_t highestAvailablePageNumber;
                    static size_t lowestAvailablePageNumber = UINT32_MAX;

                    extern "C" int __kernel_start;
                    extern "C" int __kernel_end;

                    void init();
                    void *alloc(int n);
                    void free(const void *pages, int n);
                    static void mark(size_t pageNumber, int n, bool used);

                    void init() {
                        libk::memset(memoryPageTable, 0xff, 32 * 4096);

                        const mmap::entry_t *mmap = mmap::get();
                        size_t mmap_length = mmap::getLength();

                        for(size_t i = 0; i < mmap_length; i++) {
                            if(mmap[i].type == 1) {
                                uint64_t roundedBase = (mmap[i].base + 0xfff) & 0xfffffffffffff000;
                                uint64_t roundedLength = (mmap[i].length - (roundedBase - mmap[i].base)) & 0xfffffffffffff000;
                                uint64_t end = roundedBase + roundedLength;

                                if(roundedBase < end) {
                                    free((const void *)(size_t)roundedBase, roundedLength >> 12);
                                    highestAvailablePageNumber = (end - 1) >> 12;

                                    size_t pageNumber = roundedBase >> 12;

                                    if(lowestAvailablePageNumber > pageNumber) {
                                        lowestAvailablePageNumber = pageNumber;
                                    }
                                }
                            }
                        }

                        // Mark kernel pages as used
                        size_t kernel_size = ((size_t)&__kernel_end) - ((size_t)&__kernel_start);
                        size_t kernel_pages = (kernel_size + 0xfff) >> 12;
                        size_t pageNumber = (((size_t)&__kernel_start) - 0xc0000000) >> 12;
                        mark(pageNumber, kernel_pages, true);

                        // Mark < 1MB addresses as used
                        mark(0, 256, true);
                    }

                    void *alloc(int n) {
                        if(n == 0) {
                            return NULL;
                        }

                        static size_t pageNumber = 0;
                        size_t pageCount = 0;
                        size_t firstAvailablePage = pageNumber;

                        size_t firstPageNumber = pageNumber;
                        size_t memoryTablePageIndex = pageNumber >> 3;
                        size_t memoryTablePageByteShift = pageNumber & 0x07;

                        do {
                            // Get page entry
                            uint8_t pageEntry = memoryPageTable[memoryTablePageIndex] & (0x80 >> memoryTablePageByteShift);
                            
                            if(pageEntry) {
                                pageCount = 0;
                            } else {
                                if(pageCount == 0) {
                                    firstAvailablePage = pageNumber;
                                }

                                pageCount++;
                            }

                            memoryTablePageByteShift++;

                            if(memoryTablePageByteShift == 8) {
                                memoryTablePageIndex++;
                                memoryTablePageByteShift = 0;
                            }

                            pageNumber++;

                            if((pageCount < (size_t)n) && (pageNumber > highestAvailablePageNumber)) {
                                pageCount = 0;
                                pageNumber = lowestAvailablePageNumber;
                                memoryTablePageIndex = pageNumber >> 3;
                                memoryTablePageByteShift = pageNumber & 0x07;
                            }
                        } while((pageNumber != firstPageNumber) && (pageCount < (size_t)n));

                        if(pageCount == (size_t)n) {
                            mark(firstAvailablePage, pageCount, true);
                            return (void *)(firstAvailablePage << 12);
                        } else {
                            return NULL;
                        }
                    }

                    void free(const void *pages, int n) {
                        size_t pageNumber = ((size_t)pages) >> 12;
                        mark(pageNumber, n, false);
                    }

                    static void mark(size_t pageNumber, int n, bool used) {
                        size_t memoryTablePageIndex = pageNumber >> 3;
                        size_t memoryTablePageByteShift = pageNumber & 0x07;

                        for(int i = 0; i < n; i++) {
                            if(used) {
                                memoryPageTable[memoryTablePageIndex] |= 0x80 >> memoryTablePageByteShift;
                            } else {
                                memoryPageTable[memoryTablePageIndex] &= ~(0x80 >> memoryTablePageByteShift);
                            }
                            
                            memoryTablePageByteShift++;

                            if(memoryTablePageByteShift == 8) {
                                memoryTablePageIndex++;
                                memoryTablePageByteShift = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}
