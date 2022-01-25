#ifndef __INC_ARCH_X86_MMU_H__
#define __INC_ARCH_X86_MMU_H__

// =============================================================================
// File inclusion
// =============================================================================
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// =============================================================================
// Public functions declaration
// =============================================================================
/**
 * @brief Initializes the MMU module.
 */
void mmuInit(void);

/**
 * @brief Allocates n consecutive frames. These frames will not be mapped in the
 *        address space by this function.
 *
 * @param[in] p_nbFrames The number of consecutive frames to allocate.
 *
 * @returns A pointer to the physical address of the first allocated frame.
 * @retval NULL if an error occurred.
 */
void *mmuAllocateFrames(int p_nbFrames);

/**
 * @brief Allocates n consecutive frames starting from the given frame.
 *
 * @param[in] p_frame The physical address of the first frame.
 * @param[in] p_nbFrames The number of consecutive frames to allocate.
 */
void mmuAllocateFramesAt(const void *p_frame, int p_nbFrames);

/**
 * @brief Frees n consecutive frames starting from the given frame.
 *
 * @param[in] p_frame The physical address of the first frame.
 * @param[in] p_nbFrames The number of consecutive frames to free.
 */
void mmuFreeFramesAt(const void *p_frame, int p_nbFrames);

/**
 * @brief Allocates n consecutive pages in the address space. These pages will
 *        be marked as non-present (unmapped).
 *
 * @param[in] p_nbPages The number of consecutive pages to allocate.
 * @param[in] p_kernel true if the pages should be allocated in kernel space (
 *                     with an address >= 0xc0000000), false otherwise.
 *
 * @returns A pointer to the linear address of the first allocated page.
 * @retval NULL if an error occurred.
 */
void *mmuAllocatePages(int p_nbPages, bool p_kernel);

/**
 * @brief Allocates n consecutive pages in the address space starting from the
 *        given page. These pages will be marked as non-present (unmapped).
 *
 * @param[in] p_page The linear address of the starting page.
 * @param[in] p_nbPages The number of consecutive pages to allocate.
 */
void mmuAllocatePagesAt(const void *p_page, int p_nbPages);

/**
 * @brief Frees n consecutive pages in the address space starting from the
 *        given page.
 *
 * @param[in] p_page The linear address of the starting page.
 * @param[in] p_nbPages The number of consecutive pages to free.
 */
void mmuFreePagesAt(const void *p_page, int p_nbPages);

/**
 * @brief Allocates n frames and maps them into address space. The allocated
 *        frames are not guaranteeed to be consecutive. However, the mapped
 *        pages will be consecutive.
 *
 * @param[in] p_nbFrames The number of frames to allocate and map.
 * @param[in] p_kernel true if the frames shall be mapped to kernel address
 *                     space, false otherwise.
 *
 * @returns A pointer to the first allocated page.
 * @retval NULL if an error occurred.
 */
void *mmuAllocateFramesAndMap(int p_nbFrames, bool p_kernel);

/**
 * @brief Allocates n frames and maps them into address space at the given
 *        address. The allocated frames are not guaranteeed to be consecutive.
 *        However, the mapped pages will be consecutive.
 * @details The pages are marked as allocated.
 *
 * @param[in] p_page A pointer to the linear address of the first page (= where
 *                   to map the allocated frames).
 * @param[in] p_nbFrames The number of frames to allocate and map.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if no error occurred.
 * @retval 1 if an error occurred.
 */
int mmuAllocateFramesAndMapAt(const void *p_page, int p_nbFrames);

/**
 * @brief Maps n consecutive frames into the address space.
 * @details The pages are marked as allocated.
 *
 * @param[in] p_frame The physical address of the first frame to map.
 * @param[in] p_nbFrames The number of consecutive frames to map.
 * @param[in] p_kernel true if the frames shall be mapped into the kernel
 *                     address space, false otherwise.
 *
 * @returns A pointer to the first allocated page.
 * @retval NULL if an error occurred.
 */
void *mmuMapFrames(const void *p_frame, int p_nbFrames, bool p_kernel);

/**
 * @brief Maps n consecutive frames into the address space at the given address.
 * @details The pages are marked as allocated.
 *
 * @param[in] p_frame The physical address of the first frame to map.
 * @param[in] p_page The linear address of the first page to map.
 * @param[in] p_nbFrames The number of consecutive frames to map.
 *
 * @returns An integer that indicates the result of the opration.
 * @retval 0 if no error occurred.
 * @retval Any other value if an error occurred.
 */
int mmuMapFramesAt(const void *p_frame, const void *p_page, int p_nbFrames);

/**
 * @brief Unmaps n consecutive pages from the address space at the given
 *        address.
 * @details The pages are unmapped but not freed.
 *
 * @param[in] p_page The linear address of the first page to unmap.
 * @param[in] p_nbPages The number of consecutive pages to unmap.
 */
void mmuUnmapPagesAt(const void *p_page, int p_nbPages);

#endif // __INC_ARCH_X86_MMU_H__
