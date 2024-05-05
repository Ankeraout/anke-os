#include "kernel/printk.h"
#include "kernel/mm/pmm.h"

#include "kernel/arch/x86_64/asm.h"
#include "kernel/arch/x86_64/gdt.h"
#include "kernel/arch/x86_64/idt.h"
#include "kernel/arch/x86_64/isr.h"
#include "kernel/arch/x86_64/pic.h"
#include "kernel/arch/x86_64/tss.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"
#include "kernel/panic.h"

#define C_KERNEL_STACK_SIZE 65536

static void ring3(void);
static void ring3_task(void);

void main(void) {
    printk("AnkeOS kernel\n");

    gdtInit();
    isrInit();
    idtInit();
    picInit();

    if(vmmInit() != 0) {
        panic("kernel: VMM initialization failed.\n");
    }

    // Create kernel stacks for TSS
    for(int l_i = 0; l_i < C_TSS_COUNT; l_i++) {
        void *l_kernelStack = pmmAlloc(C_KERNEL_STACK_SIZE);

        if(l_kernelStack == NULL) {
            panic("kernel: Failed to allocate memory for kernel stack.\n");
        }

        tssInit(&g_tss[l_i], l_kernelStack);
    }

    picEnableIrq(0);

    sti();

    pr_info("kernel: Initialization complete.\n");

    ring3();
}

static void ring3(void) {
    // Load TSS for current core
    asm(
        "movw $0x30, %ax \n"
        "ltrw %ax \n"
    );

    // Start user mode task
    asm(
        "movw $0x23, %ax \n"
        "movw %ax, %ds \n"
        "movw %ax, %es \n"
        "movw %ax, %fs \n"
        "movw %ax, %gs \n"
        "movq %rsp, %rax \n"
        "pushq $0x23 \n"
        "pushq %rax \n"
        "pushfq \n"
        "pushq $0x1b \n"
        "pushq $ring3_task \n"
        "iretq"
    );
}

static void ring3_task(void) {
    asm(
        "int $0x80 \n"
        "int $0x80 \n"
    );

    while(1) {
    }
}
