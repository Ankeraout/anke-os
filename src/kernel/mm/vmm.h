#ifndef __VMM_H__
#define __VMM_H__

/**
 *  Summary:
 *      Maps the page in physical address space at address paddr in the virtual
 *      address space at address vaddr.
 * 
 *  Args:
 *      - paddr: The physical address of the page to map
 *      - vaddr: The virtual address of the page to map
 * 
 *  Returns:
 *      0 if the mapping was successful, 1 if another page was already mapped
 *      at this virtual address or if the virtual memory manager could not map
 *      the page at the given virtual address.
 */
int vmm_map(const void *paddr, const void *vaddr);

/**
 *  Summary:
 *      Unmaps the page at virtual address vaddr in the virtual address space.
 * 
 *  Args:
 *      - vaddr: The virtual address of the page to unmap
 * 
 *  Returns:
 *      0 if the unmapping was successful, 1 if no page was mapped at this
 *      virtual address.
 */
int vmm_unmap(const void *vaddr);

/**
 *  Summary:
 *      Allocates the given number of pages contiguously in the virtual address
 *      space.
 * 
 *  Args:
 *      - pageCount: The number of contiguous pages to allocate.
 * 
 *  Returns:
 *      The address in the virtual address space of the allocated pages, or
 *      NULL if an error occurred. An error can occur in these situations:
 *          - There are no more physical pages to allocate
 *          - There is not enough contiguous space in the virtual address space
 */ 
void *vmm_alloc(int pageCount);

/**
 *  Summary:
 *      Frees the previously allocated pages by unmapping them from the virtual
 *      address space and freeing the pages in the physical memory allocator.
 * 
 *  Args:
 *      - addr: The address of the previously allocated pages
 */
void vmm_free(const void *addr);

#endif
