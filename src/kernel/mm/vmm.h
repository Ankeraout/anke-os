#ifndef __VMM_H__
#define __VMM_H__

/**
 *  Summary:
 *      Maps n pages from the physical address space at address paddr into the
 *      virtual address space at an unknown address. A call to this function
 *      is equivalent to doing a call to vmm_alloc(), then vmm_map2().
 * 
 *  Args:
 *      - paddr: The physical address of the page to map
 *      - n: Number of pages to map
 * 
 *  Returns:
 *      A pointer to the mapped area or NULL if an error occurred.
 * 
 *  Error cases:
 *      - There is not enough contiguous space in the virtual memory address
 *          space.
 */
void *vmm_map(const void *paddr, size_t n);

/**
 *  Summary:
 *      Maps n pages from the physical address space at address paddr into the
 *      virtual address space at address vaddr.
 * 
 *  Args:
 *      - paddr: The physical address of the first page to map
 *      - vaddr: The virtual address of the first page to map
 *      - n: Number of pages to map
 * 
 *  Returns:
 *      vaddr if successful, NULL if an error occurred.
 * 
 *  Error cases:
 *      - Tried to map over already mapped pages in virtual memory address
 *          space
 */
void *vmm_map2(const void *paddr, void *vaddr, size_t n);

/**
 *  Summary:
 *      Unmaps allocated pages at virtual address vaddr in the virtual address
 *      space.
 * 
 *  Args:
 *      - vaddr: The virtual address of the first page to unmap
 * 
 *  Returns:
 *      0 if the unmapping was successful, 1 otherwise.
 */
int vmm_unmap(const void *vaddr);

/**
 *  Summary:
 *      Unmaps n pages at virtual address vaddr in the virtual address space.
 * 
 *  Args:
 *      - vaddr: The virtual address of the first page to unmap
 *      - n: Number of pages to unmap
 * 
 *  Returns:
 *      0 if the unmapping was successful, 1 if the request implies to unmap
 *      already unmapped pages.
 */
int vmm_unmap2(const void *vaddr, size_t n);

/**
 *  Summary:
 *      Allocates the given number of pages contiguously in the virtual address
 *      space at a random address and returns this address, without mapping any
 *      physical page into the allocated virtual pages.
 * 
 *  Args:
 *      - n: The number of contiguous pages to allocate.
 * 
 *  Returns:
 *      The address in the virtual address space of the allocated pages, or
 *      NULL if an error occurred. An error can occur in these situations:
 *          - There is not enough contiguous space in the virtual address space
 */ 
void *vmm_alloc(size_t n);

/**
 *  Summary:
 *      Frees pages at virtual address vaddr. If the pages are mapped, then
 *      this function will also unmap them.
 * 
 *  Args:
 *      - vaddr: The virtual address of the first page to free
 * 
 *  Returns:
 *      0 if the unmapping was successful, 1 if the request implies to free
 *      non-allocated pages.
 */
int vmm_free(const void *vaddr);

#endif
