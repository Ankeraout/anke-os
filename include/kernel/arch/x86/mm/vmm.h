#ifndef __KERNEL_ARCH_X86_MM_VMM_H__
#define __KERNEL_ARCH_X86_MM_VMM_H__

#include <stdbool.h>
#include <stddef.h>

// Initializes virtual memory manager
void vmm_init();

// Maps n pages at physical address paddr to virtual address space
void *vmm_map(const void *paddr, size_t n, bool high);

// Maps n pages at physical address paddr to virtual address vaddr
void *vmm_map2(const void *paddr, void *vaddr, size_t n);

// Unmaps n pages at virtual address vaddr but don't release the physical pages
// to physical memory manager.
int vmm_unmap(const void *vaddr, size_t n);

// Unmaps n pages at virtual address vaddr and release the physical pages to
// physical memory manager.
int vmm_unmap2(const void *vaddr, size_t n);

// Allocates n pages in virtual memory space.
void *vmm_alloc(size_t n, bool high);

// Frees n pages in virtual memory space. If the pages were mapped, they are
// unmapped but not released to physical memory manager.
int vmm_free(const void *vaddr, size_t n);

#endif
