#include "boot/loader/drivers/block/block.h"
#include "boot/loader/stdio.h"
#include "boot/loader/string.h"

#define C_BLOCK_POOL_SIZE 16

static struct ts_block s_blockPool[C_BLOCK_POOL_SIZE];

int block_init(void) {
    memset(s_blockPool, 0, sizeof(s_blockPool));
    return 0;
}

struct ts_block *block_alloc(void) {
    for(int l_index = 0; l_index < C_BLOCK_POOL_SIZE; l_index++) {
        if(!s_blockPool[l_index].allocated) {
            s_blockPool[l_index].allocated = true;
            return &s_blockPool[l_index];
        }
    }

    return NULL;
}

int block_register(struct ts_block *p_block) {
    p_block->registered = true;

    printf("block: Registered %s\n", p_block->name);

    return 0;
}
