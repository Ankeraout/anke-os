#include "boot/loader/drivers/console/console.h"

#define C_MAX_CONSOLE_COUNT 4

static struct ts_console s_consolePool[C_MAX_CONSOLE_COUNT];

void console_init(void) {
    // Mark all consoles as unallocated
    for(int l_index = 0; l_index < C_MAX_CONSOLE_COUNT; l_index++) {
        s_consolePool[l_index].id = -1;
    }
}

struct ts_console *console_alloc(void) {
    for(int l_index = 0; l_index < C_MAX_CONSOLE_COUNT; l_index++) {
        if(s_consolePool[l_index].id == -1) {
            s_consolePool[l_index].enabled = false;
            s_consolePool[l_index].id = l_index;
            return &s_consolePool[l_index];
        }
    }

    return NULL;
}

int console_register(struct ts_console *p_console) {
    p_console->enabled = true;

    return 0;
}

ssize_t console_write(const void *p_buffer, size_t p_size) {
    for(int l_index = 0; l_index < C_MAX_CONSOLE_COUNT; l_index++) {
        struct ts_console *l_console = &s_consolePool[l_index];

        if(
            (l_console->id == l_index)
            && l_console->enabled
            && (l_console->write != NULL)
        ) {
            l_console->write(l_console, p_buffer, p_size);
        }
    }

    return (ssize_t)p_size;
}
