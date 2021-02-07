#include "time.h"
#include "arch/arch.h"

void sleep(unsigned int milliseconds);

void sleep(unsigned int milliseconds) {
    uint64_t endTime = kernel_timer + milliseconds;

    while(kernel_timer < endTime) {
        arch_halt();
    }
}
