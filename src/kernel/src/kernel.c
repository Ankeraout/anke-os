#include "limine.h"

static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

void _start(void) {
    terminal_request.response->write(terminal_request.response->terminals[0], "Hello world!", 12);
    while(1);
}