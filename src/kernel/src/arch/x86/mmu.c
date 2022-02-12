// =============================================================================
// File inclusion
// =============================================================================
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/x86/mmu.h"

// =============================================================================
// Private constants definition
// =============================================================================
#define C_PAGE_SIZE_BYTES 0x1000
#define C_NB_PAGES_TOTAL 0x100000
#define C_NB_BITS_PER_BYTE 8
#define C_PAGEDIRECTORY_NB_ENTRIES 1024
#define C_PAGETABLE_NB_ENTRIES 1024

enum {
    E_FMAP_NOT_AVAILABLE,
    E_FMAP_AVAILABLE
};

enum {
    E_MMAP_TYPE_AVAILABLE = 1,
    E_MMAP_TYPE_ACPI = 3,
    E_MMAP_TYPE_PRESERVE_ON_HIBERNATION = 4,
    E_MMAP_TYPE_DEFECTIVE = 5
};

// =============================================================================
// Private types declaration
// =============================================================================
/**
 * @brief This structure defines a page directory entry.
 */
typedef struct {
    uint32_t present : 1;
    uint32_t readWrite : 1;
    uint32_t user : 1;
    uint32_t writeThrough : 1;
    uint32_t cacheDisable : 1;
    uint32_t accessed : 1;
    uint32_t available : 1;
    uint32_t pageSize : 1;
    uint32_t available2 : 4;
    uint32_t frameIndex : 20;
} __attribute__((packed)) t_pageDirectoryEntry;

/**
 * @brief This structure defines a page table entry.
 */
typedef struct {
    uint32_t present : 1;
    uint32_t readWrite : 1;
    uint32_t user : 1;
    uint32_t writeThrough : 1;
    uint32_t cacheDisable : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t attributeTable : 1;
    uint32_t global : 1;
    uint32_t available : 3;
    uint32_t frameIndex : 20;
} __attribute__((packed)) t_pageTableEntry;

/**
 * @brief This structure defines a multiboot 2 memory map entry.
 */
typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed)) t_multiboot2MemoryMapEntry;

/**
 * @brief This type represents a page directory.
 */
typedef t_pageDirectoryEntry t_pageDirectory[C_PAGEDIRECTORY_NB_ENTRIES];

/**
 * @brief This type represents a page table.
 */
typedef t_pageTableEntry t_pageTable[C_PAGETABLE_NB_ENTRIES];

// =============================================================================
// Private variables declaration
// =============================================================================
static uint8_t s_frameMap[C_NB_PAGES_TOTAL / C_NB_BITS_PER_BYTE];
extern t_pageDirectoryEntry g_kernelPageDirectory[C_PAGEDIRECTORY_NB_ENTRIES];
extern t_pageTableEntry g_kernelSelfMapPageTable[C_PAGETABLE_NB_ENTRIES];
extern t_pageTable g_kernelPageTables[C_PAGEDIRECTORY_NB_ENTRIES];
extern t_multiboot2MemoryMapEntry g_kernelMemoryMap[];
extern uint32_t g_kernelMemoryMapSize;
extern int g_kernelStart;
extern int g_kernelEnd;

// =============================================================================
// Private functions declaration
// =============================================================================
/**
 * @brief Mark n frames starting from the given frame with the given status.
 *
 * @param[in] p_frame The first frame to mark.
 * @param[in] p_nbFrames The number of frames to mark.
 * @param[in] p_available The status of the frames to mark.
 */
static void mmuMarkFrames(
    const void *p_frame,
    int p_nbFrames,
    bool p_available
);

/**
 * @brief Returns the status of the given frame.
 *
 * @param[in] p_frame The frame to check.
 *
 * @returns A boolean value that indicates whether or not the given frame is
 *          available.
 * @retval true if the given frame is available.
 * @retval false if the given frame is not available.
 */
static bool mmuIsFrameAvailable(const void *p_frame);

/**
 * @brief Mark n pages starting from the given frame with the given status.
 *
 * @param[in] p_page The first page to mark.
 * @param[in] p_nbFrames The number of pages to mark.
 * @param[in] p_available The status of the pages to mark.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if no error occurred.
 * @retval Any other value if an error occurred.
 */
static int mmuMarkPages(const void *p_page, int p_nbPages, bool p_available);

/**
 * @brief Returns a boolean value that indicates whether the given page is
 *        mapped or not.
 *
 * @param[in] p_page The page to check.
 *
 * @returns A boolean value that indicates whether the page is mapped or not.
 * @retval true if the page is mapped.
 * @retval false if the page is not mapped.
 */
//static bool mmuIsPageMapped(const void *p_page);

/**
 * @brief Returns a boolean value that indicates whether the given page is free
 *        or not.
 *
 * @param[in] p_page The page to check.
 *
 * @returns A boolean value that indicates whether the page is free or not.
 * @retval true if the page is free.
 * @retval false if the page is not free.
 */
static bool mmuIsPageFree(const void *p_page);

/**
 * @brief Returns the status of the given page.
 *
 * @param[in] p_page The page to check.
 *
 * @returns A boolean value that indicates whether or not the given page is
 *          available.
 * @retval true if the given page is available.
 * @retval false if the given page is not available.
 */
static bool mmuIsPageAvailable(const void *p_page);

/**
 * @brief Creates a new page table.
 *
 * @param[in] p_pageDirectoryIndex Index of the new page table in the page
 *                                 directory.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if no error occurred.
 * @retval Any other value if an error occurred.
 */
static int mmuCreatePageTable(uint32_t p_pageDirectoryIndex);

// =============================================================================
// Public functions definition
// =============================================================================
void mmuInit(void) {
    int l_memoryMapLength = g_kernelMemoryMapSize
        / sizeof(t_multiboot2MemoryMapEntry);

    // Mark all frames as not available by default
    for(
        int l_frameMapIndex = 0;
        l_frameMapIndex < (int)sizeof(s_frameMap);
        l_frameMapIndex++
    ) {
        s_frameMap[l_frameMapIndex] = 0;
    }

    // Unmark available memory
    for(
        int l_memoryMapIndex = 0;
        l_memoryMapIndex < l_memoryMapLength;
        l_memoryMapIndex++
    ) {
        t_multiboot2MemoryMapEntry *l_memoryMapEntry =
            &g_kernelMemoryMap[l_memoryMapIndex];

        if(l_memoryMapEntry->type == E_MMAP_TYPE_AVAILABLE) {
            mmuMarkFrames(
                (const void *)((uint32_t)l_memoryMapEntry->base),
                ((int)l_memoryMapEntry->length) >> 12,
                E_FMAP_AVAILABLE
            );
        }
    }

    // Mark low memory
    mmuMarkFrames(
        (const void *)0x00000000,
        256,
        E_FMAP_NOT_AVAILABLE
    );

    // Unmark all kernel pages
    mmuMarkPages(
        (const void *)0xc0000000,
        1024,
        true
    );

    // Mark kernel pages
    size_t l_kernelStartAddress = (size_t)&g_kernelStart;
    size_t l_kernelEndAddress = (size_t)&g_kernelEnd;
    size_t l_kernelSizeBytes = l_kernelEndAddress - l_kernelStartAddress;
    size_t l_kernelSizePages = (l_kernelSizeBytes + 0xfff) >> 12;

    mmuMarkPages(
        &g_kernelStart,
        l_kernelSizePages,
        false
    );

    // Mark self-mapped pages
    mmuMarkPages(
        g_kernelPageTables,
        C_PAGEDIRECTORY_NB_ENTRIES,
        false
    );
}

void *mmuAllocateFrames(int p_nbFrames) {
    int l_searchCurrentIndex = 0;
    bool l_found = false;
    int l_firstFrame;

    while(
        (l_searchCurrentIndex < C_NB_PAGES_TOTAL)
        && !l_found
    ) {
        l_firstFrame = l_searchCurrentIndex;
        int l_nbConsecutiveFrames = 0;
        bool l_abort = false;

        while(
            (l_nbConsecutiveFrames < p_nbFrames)
            && (l_searchCurrentIndex < C_NB_PAGES_TOTAL)
            && !l_abort
        ) {
            void *l_framePtr = (void *)(l_searchCurrentIndex << 12);

            if(mmuIsFrameAvailable(l_framePtr)) {
                l_nbConsecutiveFrames++;
            } else {
                l_abort = true;
            }

            l_searchCurrentIndex++;
        }

        l_found = (l_nbConsecutiveFrames < p_nbFrames)
            && (l_searchCurrentIndex < C_NB_PAGES_TOTAL);
    }

    if(l_found) {
        void *l_framePtr = (void *)(l_firstFrame << 12);
        mmuMarkFrames(l_framePtr, p_nbFrames, E_FMAP_NOT_AVAILABLE);
        return l_framePtr;
    } else {
        return NULL;
    }
}

void mmuAllocateFramesAt(const void *p_frame, int p_nbFrames) {
    mmuMarkFrames(p_frame, p_nbFrames, E_FMAP_NOT_AVAILABLE);
}

void mmuFreeFramesAt(const void *p_frame, int p_nbFrames) {
    mmuMarkFrames(p_frame, p_nbFrames, E_FMAP_AVAILABLE);
}

void *mmuAllocatePages(int p_nbPages, bool p_kernel) {
    int l_searchStartIndex;
    int l_searchSpaceSize;
    int l_searchEndIndex;
    int l_searchCurrentIndex;
    int l_firstPage;
    bool l_found = false;

    if(p_kernel) {
        l_searchStartIndex = 0xc0000;
        l_searchSpaceSize = C_NB_PAGES_TOTAL * 1 / 4;
    } else {
        l_searchStartIndex = 0x00000;
        l_searchSpaceSize = C_NB_PAGES_TOTAL * 3 / 4;
    }

    l_searchEndIndex = l_searchStartIndex + l_searchSpaceSize;
    l_searchCurrentIndex = l_searchStartIndex;

    while(
        (l_searchCurrentIndex < l_searchEndIndex)
        && !l_found
    ) {
        l_firstPage = l_searchCurrentIndex;
        int l_nbConsecutivePages = 0;
        bool l_abort = false;

        while(
            (l_nbConsecutivePages < p_nbPages)
            && (l_searchCurrentIndex < l_searchEndIndex)
            && !l_abort
        ) {
            void *l_pagePtr = (void *)(l_searchCurrentIndex << 12);

            if(mmuIsPageAvailable(l_pagePtr)) {
                l_nbConsecutivePages++;
            } else {
                l_abort = true;
            }

            l_searchCurrentIndex++;
        }

        l_found = (l_nbConsecutivePages == p_nbPages)
            && (l_searchCurrentIndex < l_searchEndIndex);
    }

    if(l_found) {
        void *l_pagePtr = (void *)(l_firstPage << 12);
        mmuMarkPages(l_pagePtr, p_nbPages, E_FMAP_NOT_AVAILABLE);
        return l_pagePtr;
    } else {
        return NULL;
    }
}

void mmuAllocatePagesAt(const void *p_page, int p_nbPages) {
    mmuMarkPages(p_page, p_nbPages, E_FMAP_NOT_AVAILABLE);
}

void mmuFreePagesAt(const void *p_page, int p_nbPages) {
    mmuMarkPages(p_page, p_nbPages, E_FMAP_AVAILABLE);
}

void *mmuAllocateFramesAndMap(int p_nbFrames, bool p_kernel) {
    void *l_pages = mmuAllocatePages(p_nbFrames, p_kernel);

    if(l_pages == NULL) {
        return NULL;
    }

    int l_returnValue = mmuAllocateFramesAndMapAt(l_pages, p_nbFrames);

    if(l_returnValue != 0) {
        mmuFreePagesAt(l_pages, p_nbFrames);
        return NULL;
    }

    return l_pages;
}

int mmuAllocateFramesAndMapAt(const void *p_page, int p_nbFrames) {
    bool l_error = false;
    int l_remainingFrames = p_nbFrames;

    uint32_t l_currentPageAddress = (uint32_t)p_page;

    while((l_remainingFrames > 0) && !l_error) {
        void *l_allocatedFrame = mmuAllocateFrames(1);

        if(l_allocatedFrame == NULL) {
            l_error = true;
        } else {
            l_error = mmuMapFramesAt(
                l_allocatedFrame,
                (const void *)l_currentPageAddress,
                1
            );

            l_currentPageAddress += C_PAGE_SIZE_BYTES;
        }
    }

    if(l_error) {
        // TODO: free allocated frames
        mmuUnmapPagesAt(p_page, p_nbFrames);
        return 1;
    }

    return 0;
}

void *mmuMapFrames(const void *p_frame, int p_nbFrames, bool p_kernel) {
    void *l_pages = mmuAllocatePages(p_nbFrames, p_kernel);

    if(l_pages == NULL) {
        return NULL;
    }

    int l_returnValue = mmuMapFramesAt(p_frame, l_pages, p_nbFrames);

    if(l_returnValue != 0) {
        mmuFreePagesAt(l_pages, p_nbFrames);
        return NULL;
    }

    return l_pages;
}

int mmuMapFramesAt(const void *p_frame, const void *p_page, int p_nbFrames) {
    int l_remainingFrames = p_nbFrames;

    uint32_t l_pageDirectoryIndex = ((uint32_t)p_page) >> 22;
    uint32_t l_pageTableIndex = (((uint32_t)p_page) >> 12) & 0x000003ff;
    uint32_t l_currentFrameIndex = ((uint32_t)p_frame) >> 12;
    bool l_error = false;

    while((l_remainingFrames > 0) && !l_error) {
        // If the page directory entry is not present, create it.
        if(g_kernelPageDirectory[l_pageDirectoryIndex].present == 0) {
            int l_returnValue = mmuCreatePageTable(l_pageDirectoryIndex);

            if(l_returnValue != 0) {
                l_error = true;
            }
        }

        g_kernelPageTables[l_pageDirectoryIndex][l_pageTableIndex].present = 1;
        g_kernelPageTables[l_pageDirectoryIndex][l_pageTableIndex].available = 0;
        g_kernelPageTables[l_pageDirectoryIndex][l_pageTableIndex].frameIndex =
            l_currentFrameIndex;

        l_currentFrameIndex++;
        l_pageTableIndex++;

        if(l_pageTableIndex >= C_PAGETABLE_NB_ENTRIES) {
            l_pageDirectoryIndex++;
            l_pageTableIndex = 0;
        }

        l_remainingFrames--;
    }

    if(l_error) {
        mmuUnmapPagesAt(p_page, p_nbFrames);
        return 1;
    }

    return 0;
}

void mmuUnmapPagesAt(const void *p_page, int p_nbPages) {
    int l_remainingPages = p_nbPages;

    uint32_t l_pageDirectoryIndex = ((uint32_t)p_page) >> 22;
    uint32_t l_pageTableIndex = (((uint32_t)p_page) >> 12) & 0x000003ff;

    while(l_remainingPages > 0) {
        // If the page directory entry is not present, the page is already
        // unmapped.
        if(g_kernelPageDirectory[l_pageDirectoryIndex].present != 0) {
            // Unmap the page.
            g_kernelPageTables[l_pageDirectoryIndex][l_pageTableIndex].present
                = 0;
        }

        l_pageTableIndex++;

        if(l_pageTableIndex >= C_PAGETABLE_NB_ENTRIES) {
            l_pageTableIndex = 0;
            l_pageDirectoryIndex++;
        }

        if(l_pageDirectoryIndex >= C_PAGEDIRECTORY_NB_ENTRIES) {
            l_pageDirectoryIndex = 0;
        }

        l_remainingPages--;
    }
}

// =============================================================================
// Private functions definition
// =============================================================================
static void mmuMarkFrames(
    const void *p_frame,
    int p_nbFrames,
    bool p_available
) {
    uint32_t l_frameIndex = ((uint32_t)p_frame) >> 12;

    for(
        int l_nbProcessedFrames = 0;
        l_nbProcessedFrames < p_nbFrames;
        l_nbProcessedFrames++
    ) {
        uint32_t l_frameMapIndex = l_frameIndex >> 3;
        uint32_t l_byteIndex = l_frameIndex & 0x00000007;
        uint8_t l_mask = 1 << (7 - l_byteIndex);

        if(p_available) {
            s_frameMap[l_frameMapIndex] |= l_mask;
        } else {
            s_frameMap[l_frameMapIndex] &= ~l_mask;
        }

        l_frameIndex++;
    }
}

static bool mmuIsFrameAvailable(const void *p_frame) {
    uint32_t l_frameIndex = ((uint32_t)p_frame) >> 12;
    uint32_t l_frameMapIndex = l_frameIndex >> 3;
    uint32_t l_byteIndex = l_frameIndex & 0x00000007;
    uint8_t l_mask = 1 << (7 - l_byteIndex);

    return ((s_frameMap[l_frameMapIndex] & l_mask) != 0);
}

static int mmuMarkPages(const void *p_page, int p_nbPages, bool p_available) {
    int l_remainingPages = p_nbPages;

    uint32_t l_pageDirectoryIndex = ((uint32_t)p_page) >> 22;
    uint32_t l_pageTableIndex = (((uint32_t)p_page) >> 12) & 0x000003ff;

    while(l_remainingPages > 0) {
        // If the page directory entry is not present, create it.
        if(g_kernelPageDirectory[l_pageDirectoryIndex].present == 0) {
            int l_returnValue = mmuCreatePageTable(l_pageDirectoryIndex);

            if(l_returnValue != 0) {
                return l_returnValue;
            }
        }

        // Mark the page.
        if(p_available) {
            g_kernelPageTables[l_pageDirectoryIndex][l_pageTableIndex]
                .available = 1;
        } else {
            g_kernelPageTables[l_pageDirectoryIndex][l_pageTableIndex]
                .available = 0;
        }

        l_pageTableIndex++;

        if(l_pageTableIndex >= C_PAGETABLE_NB_ENTRIES) {
            l_pageTableIndex = 0;
            l_pageDirectoryIndex++;
        }

        if(l_pageDirectoryIndex >= C_PAGEDIRECTORY_NB_ENTRIES) {
            l_pageDirectoryIndex = 0;
        }

        l_remainingPages--;
    }

    return 0;
}

/*
static bool mmuIsPageMapped(const void *p_page) {
    uint32_t l_pageDirectoryIndex = ((uint32_t)p_page) >> 22;
    uint32_t l_pageTableIndex = (((uint32_t)p_page) >> 12) & 0x000003ff;

    // If the page directory entry is not present, the page is available.
    if(g_kernelPageDirectory[l_pageDirectoryIndex].present == 0) {
        return false;
    }

    // Otherwise we return the status of the page table entry.
    return (
        g_kernelPageTables[l_pageDirectoryIndex][l_pageTableIndex].present == 1
    );
}
*/

static bool mmuIsPageFree(const void *p_page) {
    uint32_t l_pageDirectoryIndex = ((uint32_t)p_page) >> 22;
    uint32_t l_pageTableIndex = (((uint32_t)p_page) >> 12) & 0x000003ff;

    // If the page directory entry is not present, the page is free.
    if(g_kernelPageDirectory[l_pageDirectoryIndex].present == 0) {
        return true;
    }

    // Otherwise we return the status of the page table entry.
    return (
        g_kernelPageTables[l_pageDirectoryIndex][l_pageTableIndex].available
            == 1
    );
}

static bool mmuIsPageAvailable(const void *p_page) {
    return mmuIsPageFree(p_page);
}

static int mmuCreatePageTable(uint32_t p_pageDirectoryIndex) {
    // Allocate one frame for the page table.
    const void *l_frame = mmuAllocateFrames(1);

    // If the allocation failed, return an error.
    if(l_frame == NULL) {
        return 1;
    }

    // Entry for the page directory.
    t_pageDirectoryEntry l_pageDirectoryEntry = {
        .accessed = 0,
        .available = 0,
        .available2 = 0,
        .cacheDisable = 0,
        .frameIndex = ((uint32_t)l_frame) >> 12,
        .pageSize = 0,
        .present = 1,
        .readWrite = 1,
        .user = 0,
        .writeThrough = 1
    };

    g_kernelPageDirectory[p_pageDirectoryIndex] = l_pageDirectoryEntry;

    // Entry for the self-mapping page table.
    t_pageTableEntry l_pageTableEntry = {
        .accessed = 0,
        .attributeTable = 0,
        .available = 0,
        .cacheDisable = 0,
        .dirty = 0,
        .frameIndex = ((uint32_t)l_frame) >> 12,
        .global = 0,
        .present = 1,
        .readWrite = 1,
        .user = 0,
        .writeThrough = 1
    };

    g_kernelSelfMapPageTable[p_pageDirectoryIndex] = l_pageTableEntry;

    // Initialize the page table entries
    t_pageTableEntry l_newPageTableEntry = {
        .accessed = 0,
        .attributeTable = 0,
        .available = 1,
        .cacheDisable = 0,
        .dirty = 0,
        .frameIndex = ((uint32_t)l_frame) >> 12,
        .global = 0,
        .present = 0,
        .readWrite = 1,
        .user = (p_pageDirectoryIndex < 768) ? 1 : 0,
        .writeThrough = 1
    };

    for(
        int l_entryIndex = 0;
        l_entryIndex < C_PAGETABLE_NB_ENTRIES;
        l_entryIndex++
    ) {
        g_kernelPageTables[p_pageDirectoryIndex][l_entryIndex] = l_newPageTableEntry;
    }

    // Success, return 0.
    return 0;
}
