#include <stdbool.h>

#include "arch/i686/bioscall.h"
#include "arch/i686/mmap.h"
#include "libk/stdio.h"
#include "libk/string.h"

#define MAX_MMAP_LENGTH 100
#define MMAP_BUFFER_ADDRESS 0x10000
#define MMAP_BUFFER_SEGMENT (MMAP_BUFFER_ADDRESS >> 4)
#define MMAP_BUFFER_OFFSET (MMAP_BUFFER_ADDRESS & 0xffff)

mmap_entry_t mmap_buffer[MAX_MMAP_LENGTH];
int mmap_length = 0;

void mmap_init();
static int mmap_init_e820();
static int mmap_init_e801();
static void mmap_arrange();
static void mmap_remove(int index);
static void mmap_fix();
static void mmap_sort();
const mmap_entry_t *mmap_get();
int mmap_getLength();

void mmap_init() {
    if(mmap_init_e820()) {
        if(mmap_init_e801()) {
            printf("mmap: could not get memory map\n");
            printf("mmap: system halted\n");

            while(true) {
                asm("cli");
                asm("hlt");
            }
        }
    }

    mmap_arrange();

    size_t ramTotal = 0;

    for(int i = 0; i < mmap_length; i++) {
        ramTotal += mmap_buffer[i].length >> 20;
    }

    printf("mmap: detected %d MiB of usable memory\n", (int)ramTotal);
}

static int mmap_init_e820() {
    bioscall_context_t context = {
        .eax = 0xe820,
        .ebx = 0,
        .ecx = sizeof(mmap_entry_t),
        .edx = 0x534d4150,
        .es = MMAP_BUFFER_SEGMENT,
        .di = MMAP_BUFFER_OFFSET
    };

    mmap_length = 0;

    while(true) {
        context.eax = 0xe820;
        context.edx = 0x534d4150;

        bioscall(0x15, &context);

        if(context.eax != 0x534d4150) {
            printf("e820: wrong signature in EAX (expected 0x534d4150, got %#08x)\n", context.eax);
            printf("e820: failed after %d BIOS calls\n", mmap_length);
            return 1;
        }

        if(context.ecx != 20) {
            printf("e820: wrong value in ECX (expected 20, got %d)\n", context.ecx);
            return 1;
        }

        memcpy(&mmap_buffer[mmap_length], (const void *)MMAP_BUFFER_ADDRESS, sizeof(mmap_entry_t));

        mmap_length++;

        if(context.flags & BIOSCALL_CONTEXT_EFLAGS_CF) {
            return 0;
        }

        if((context.ebx == 0) || (mmap_length == MAX_MMAP_LENGTH)) {
            return 0;
        }
    }

    return 0;
}

static int mmap_init_e801() {
    bioscall_context_t context = {
        .ax = 0xe801,
        .cx = 0,
        .dx = 0
    };

    bioscall(0x15, &context);

    if(context.flags & BIOSCALL_CONTEXT_EFLAGS_CF) {
        printf("e801: BIOS call returned carry flag\n");
        return 1;
    }

    if((context.ah == 0x86) || (context.ah == 0x80)) {
        printf("e801: BIOS call returned AH=%02x\n", context.ah);
        return 1;
    }

    printf("e801: ax=%#04x bx=%#04x cx=%#04x dx=%#04x\n", context.ax, context.bx, context.cx, context.dx);

    size_t ramUnder16mb;
    size_t ramOver16mb;

    if((context.ax == 0) && (context.bx == 0)) {
        if((context.cx == 0) && (context.dx == 0)) {
            printf("e801: BIOS call returned 0 MB\n");
            return 1;
        } else {
            ramUnder16mb = context.cx * 1024;
            ramOver16mb = context.dx * 65536;
        }
    } else {
        ramUnder16mb = context.ax * 1024;
        ramOver16mb = context.bx * 65536;
    }

    mmap_length = 2;

    mmap_buffer[0].base = 0x00100000;
    mmap_buffer[0].length = ramUnder16mb;
    mmap_buffer[0].type = 1;

    mmap_buffer[1].base = 0x01000000;
    mmap_buffer[1].length = ramOver16mb;
    mmap_buffer[1].type = 1;

    return 0;
}

static void mmap_arrange() {
    mmap_sort();
    mmap_fix();
}

static void mmap_remove(int index) {
    for(int i = index; i < mmap_length - 1; i++) {
        mmap_buffer[i] = mmap_buffer[i + 1];
    }

    mmap_length--;
}

static void mmap_fix() {
    for(int i = 0; i < mmap_length - 1; i++) {
        if((mmap_buffer[i].base + mmap_buffer[i].length) > mmap_buffer[i + 1].base) {
            uint64_t lengthDiff = mmap_buffer[i].base + mmap_buffer[i].length - mmap_buffer[i + 1].base;

            bool reserved1 = (mmap_buffer[i].type != 1);
            bool reserved2 = (mmap_buffer[i + 1].type != 1);

            if(reserved1 == reserved2) {
                mmap_buffer[i].length -= lengthDiff;
            } else if(reserved1) {
                if(mmap_buffer[i + 1].length > lengthDiff) {
                    mmap_remove(i + 1);
                    i--;
                } else {
                    mmap_buffer[i + 1].base += lengthDiff;
                    mmap_buffer[i + 1].length -= lengthDiff;
                }
            } else if(reserved2) {
                if(mmap_buffer[i].length == lengthDiff) {
                    mmap_remove(i);
                    i--;
                } else {
                    mmap_buffer[i].length -= lengthDiff;
                }
            }
        }
    }
}

static void mmap_sort() {
    mmap_entry_t exchange;

    int minIndex = 0;
    uint64_t minValue = 0;

    for(int i = 0; i < mmap_length - 1; i++) {
        minIndex = i;

        for(int j = i + 1; j < mmap_length; j++) {
            if(mmap_buffer[j].base < minValue) {
                minValue = mmap_buffer[j].base;
                minIndex = j;
            }
        }

        if(minIndex != i) {
            exchange = mmap_buffer[i];
            mmap_buffer[i] = mmap_buffer[minIndex];
            mmap_buffer[minIndex] = exchange;
        }
    }
}

const mmap_entry_t *mmap_get() {
    return mmap_buffer;
}

int mmap_getLength() {
    return mmap_length;
}
