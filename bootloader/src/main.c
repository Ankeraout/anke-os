#include <stdint.h>

#include "asm.h"
#include "stdio.h"
#include "sysinfo.h"

int main(struct ts_systemInformation *p_systemInformation) {
    printf("%s\n", (const char *)p_systemInformation->m_bootloaderName);

    while(1) {
        cli();
        hlt();
    }

    return 0;
}
