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
static void mmap_arrange();
static void mmap_remove(int index);
static void mmap_fix();
static void mmap_sort();
const mmap_entry_t *mmap_get();
int mmap_getLength();

void mmap_init() {
    if(mmap_init_e820()) {
        // TODO: die
    }

    mmap_arrange();
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

    while(true) {
        context.eax = 0xe820;
        context.edx = 0x534d4150;

        bioscall(0x15, &context);

        if(context.eax != 0x534d4150) {
            return 1;
        }

        if(context.ecx != 20) {
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
