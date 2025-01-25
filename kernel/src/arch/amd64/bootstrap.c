#include <stddef.h>

#include "limine.h"

static struct limine_memmap_request s_memoryMapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct limine_rsdp_request s_rsdpRequest = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct limine_smbios_request s_smbiosRequest = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0,
    .response = NULL
};

static struct limine_framebuffer_request s_framebufferRequest = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = NULL
};

void bootstrap(void) {

}
