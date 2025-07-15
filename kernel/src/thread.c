#include "criticalSection.h"
#include "memoryAllocation.h"
#include "mm/vmm.h"
#include "stdlib.h"
#include "thread.h"

/**
 * @brief Allocates physical memory for the thread and returns the pointer to
 * it. The memory is
 * 
 * @param[in] p_size The number of bytes to allocate.
 * 
 * @returns A pointer to the physical address of the allocated memory.
 */
static void *thread_allocateMemory(struct ts_thread *p_thread, size_t p_size);

struct ts_thread *thread_new(
    struct ts_process *p_process,
    void *p_entryPoint,
    size_t p_stackSize
) {
    criticalSection_enter();

    struct ts_thread *l_thread = malloc(sizeof(struct ts_thread));

    if(l_thread == NULL) {
        goto errorFailedToAllocateThread;
    }

    struct ts_memoryAllocation *l_memoryAllocation =
        malloc(sizeof(struct ts_memoryAllocation));

    if(l_memoryAllocation == NULL) {
        goto errorFailedToAllocateMemoryAllocation;
    }

    errorFailedToAllocateMemoryAllocation:
    free(l_thread);

    errorFailedToAllocateThread:
    criticalSection_leave();
    return NULL;
}

void thread_start(struct ts_thread *p_thread) {

}

void thread_destroy(struct ts_thread *p_thread) {

}

static void *thread_allocateMemory(struct ts_thread *p_thread, size_t p_size) {
    
}
