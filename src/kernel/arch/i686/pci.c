#include "arch/i686/bioscall.h"
#include "arch/i686/io.h"

enum {
    PCI_CSAM_NONE,
    PCI_CSAM_1,
    PCI_CSAM_2
};

void pci_init();
static int pci_detectCsam();

void pci_init() {

}

static int pci_detectCsam() {
    bioscall_context_t context = {
        .ax = 0xb101,
        .edi = 0x00000000
    };

    bioscall(0x1a, &context);

    // If the BIOS call succeeded:
    //  - ah = 0x00
    //  - CF is clear
    //  - edx = 0x20494350 ("PCI ")
    //  - edi = physical pointer to protected-mode entry point
    //  - al = PCI hardware characteristics
    //  - bh = PCI interface level major version
    //  - bl = PCI interface level minor version
    //  - cl = number of last PCI bus in the system
    if(context.ah != 0x00) {
        return PCI_CSAM_NONE;
    } else if(context.eflags & BIOSCALL_CONTEXT_EFLAGS_CF) {
        return PCI_CSAM_NONE;
    } else if(context.edx != 0x20494350) {
        return PCI_CSAM_NONE;
    } else if(context.al & (1 << 0)) {
        return PCI_CSAM_1;
    } else if(context.al & (1 << 1)) {
        return PCI_CSAM_2;
    } else {
        return PCI_CSAM_NONE;
    }
}
