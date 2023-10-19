#include <stdbool.h>

#include "kernel/arch/x86_64/inline.h"
#include "kernel/arch.h"
#include "kernel/boot.h"
#include "kernel/device.h"
#include "kernel/device/system.h"
#include "kernel/fs/vfs.h"
#include "kernel/mm/pmm.h"
#include "kernel/module.h"
#include "klibc/debug.h"
#include "sys/stat.h"

static int kernelPreinit(const struct ts_kernelBootInfo *p_bootInfo);
static int kernelInit(void);

static const char *s_moduleList[] = {
    "hello",
    "ramfs"
};

void kernelMain(const struct ts_kernelBootInfo *p_bootInfo) {
    int l_returnValue = archPreinit(p_bootInfo);

    if(l_returnValue != 0) {
        kernelDebug(
            "kernel: archPreinit() failed with code %d.\n",
            l_returnValue
        );

        return;
    }

    l_returnValue = kernelPreinit(p_bootInfo);

    if(l_returnValue != 0) {
        kernelDebug(
            "kernel: kernelPreinit() failed with code %d.\n",
            l_returnValue
        );

        return;
    }

    l_returnValue = archInit();

    if(l_returnValue != 0) {
        kernelDebug("kernel: archInit() failed with code %d.\n", l_returnValue);
        return;
    }

    l_returnValue = kernelInit();

    if(l_returnValue != 0) {
        kernelDebug(
            "kernel: kernelInit() failed with code %d.\n",
            l_returnValue
        );
        
        return;
    }
}

static int kernelPreinit(const struct ts_kernelBootInfo *p_bootInfo) {
    kernelDebug("\nMemory map:\n\n");
    kernelDebug("Base               | Length             | Type\n");
    kernelDebug("-------------------+--------------------+-----\n");

    const char *l_type = NULL;

    for(size_t l_index = 0; l_index < p_bootInfo->memoryMapEntryCount; l_index++) {
        const struct ts_kernelMemoryMapEntry *l_memoryMapEntry = &p_bootInfo->memoryMap[l_index];

        switch(l_memoryMapEntry->type) {
            case E_KERNELMEMORYMAPENTRYTYPE_FREE: l_type = "Free"; break;
            case E_KERNELMEMORYMAPENTRYTYPE_RECLAIMABLE: l_type = "Reclaimable"; break;
            case E_KERNELMEMORYMAPENTRYTYPE_RESERVED: l_type = "Reserved"; break;
            default: l_type = "Unknown"; break;
        }

        kernelDebug("0x%016lx | 0x%016lx | %s\n", l_memoryMapEntry->base, l_memoryMapEntry->length, l_type);
    }

    kernelDebug("\n");

    if(pmmInit(p_bootInfo) != 0) {
        kernelDebug("PMM initialization failed.\n");
        return -1;
    }

    return 0;
}

static int kernelInit(void) {
    // Initialize VFS
    int l_returnValue = vfsInit();

    if(l_returnValue != 0) {
        kernelDebug("kernel: Failed to initialize VFS.\n");
        return l_returnValue;
    }

    // Initialize the device system
    l_returnValue = deviceInit();

    if(l_returnValue != 0) {
        kernelDebug("kernel: Failed to initialize the device system.\n");
        return l_returnValue;
    }

    // Initialize kernel devices
    l_returnValue = deviceSystemInit();

    if(l_returnValue != 0) {
        kernelDebug("kernel: Failed to initialize system devices.\n");
        return l_returnValue;
    }

    // Initialize modules
    l_returnValue = modulesInit();

    if(l_returnValue != 0) {
        kernelDebug("kernel: Failed to initialize modules.\n");
        return l_returnValue;
    }

    for(int l_i = 0; l_i < sizeof(s_moduleList) / sizeof(const char *); l_i++) {
        kernelDebug("kernel: Initializing %s module...\n", s_moduleList[l_i]);

        const struct ts_module *l_module =
            moduleGetKernelModule(s_moduleList[l_i]);

        if(l_module == NULL) {
            kernelDebug("kernel: %s module not found!\n", s_moduleList[l_i]);
            continue;
        }

        l_returnValue = l_module->m_init();

        if(l_returnValue != 0) {
            kernelDebug(
                "kernel: Initialization of module %s failed with code %d.\n",
                l_module->m_name,
                l_returnValue
            );

            continue;
        }
    }

    // Mount ramfs at /
    kernelDebug("kernel: Mounting ramfs at root...\n");

    l_returnValue = vfsMount("/", "ramfs", NULL);

    if(l_returnValue != 0) {
        kernelDebug(
            "kernel: Failed to mount ramfs at root with code %d.\n",
            l_returnValue
        );

        return l_returnValue;
    }

    kernelDebug("kernel: root mounted successfully!\n");

    // Test stuff
    kernelDebug("kernel: Creating /dev...\n");

    l_returnValue = vfsMknod("/dev", S_IFDIR | 0755, 0);

    if(l_returnValue != 0) {
        kernelDebug(
            "kernel: Failed to mkdir /dev: %d.\n",
            l_returnValue
        );

        return l_returnValue;
    }

    kernelDebug("kernel: Creating /dev/zero...\n");

    l_returnValue = vfsMknod("/dev/zero", S_IFCHR | 0755, M_DEVICE_MAKE(1, 5));

    if(l_returnValue != 0) {
        kernelDebug(
            "kernel: Failed to create /dev/zero: %d.\n",
            l_returnValue
        );

        return l_returnValue;
    }

    kernelDebug("kernel: Opening /dev/zero...\n");

    struct ts_vfsFile *l_file;

    l_returnValue = vfsOpen("/dev/zero", 0, &l_file);

    if(l_returnValue != 0) {
        kernelDebug(
            "kernel: Failed to open /dev/zero: %d.\n",
            l_returnValue
        );

        return l_returnValue;
    }

    kernelDebug("kernel: Reading 1024 bytes from /dev/zero...\n");

    uint8_t l_buffer[1024];

    l_returnValue = vfsFileRead(l_file, l_buffer, 1024);

    kernelDebug("kernel: %d bytes read.\n", l_returnValue);

    kernelDebug("kernel: Closing /dev/zero...\n");

    l_returnValue = vfsFileClose(l_file);

    return 0;
}
