#include <stdbool.h>

#include <kernel/debug.h>
#include <kernel/module.h>
#include <kernel/misc/list.h>

#include "string.h"

extern const struct ts_module g_kernelModules[];
extern int g_kernelModulesEnd;

static struct ts_arrayList s_loadedModules;

static size_t moduleGetKernelModuleCount(void);

int moduleInit(void) {
    if(arrayListInit(&s_loadedModules) != 0) {
        return 1;
    }

    const size_t l_kernelModuleCount = moduleGetKernelModuleCount();

    debug(
        "kernel: Found %d loadable kernel modules:\n",
        l_kernelModuleCount
    );

    for(size_t l_index = 0; l_index < l_kernelModuleCount; l_index++) {
        debug(
            "kernel:   - %s\n",
            g_kernelModules[l_index].a_name
        );
    }

    return 0;
}

const struct ts_module *moduleGetKernelModule(const char *p_name) {
    size_t l_index = 0;
    size_t l_moduleCount = moduleGetKernelModuleCount();

    while(l_index < l_moduleCount) {
        if(strcmp(g_kernelModules[l_index].a_name, p_name) == 0) {
            return &g_kernelModules[l_index];
        }

        l_index++;
    }

    return NULL;
}

int moduleLoad(const struct ts_module *p_module, const char *p_args) {
    if(moduleIsModuleLoaded(p_module)) {
        return 0;
    }

    int l_returnValue = arrayListAdd(&s_loadedModules, (void *)p_module);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    l_returnValue = p_module->a_init(p_args);

    if(l_returnValue != 0) {
        arrayListRemoveItem(&s_loadedModules, p_module);

        return l_returnValue;
    }

    return l_returnValue;
}

int moduleUnload(const struct ts_module *p_module) {
    if(!moduleIsModuleLoaded(p_module)) {
        return 0;
    }

    p_module->a_quit();

    return arrayListRemoveItem(&s_loadedModules, p_module);
}

size_t moduleGetLoadedModuleCount(void) {
    return arrayListGetLength(&s_loadedModules);
}

const struct ts_module *moduleGetLoadedModule(const char *p_name) {
    for(size_t l_i = 0; l_i < arrayListGetLength(&s_loadedModules); l_i++) {
        const struct ts_module *l_module =
            (const struct ts_module *)arrayListGet(&s_loadedModules, l_i);

        if(strcmp(p_name, l_module->a_name) == 0) {
            return l_module;
        }
    }

    return NULL;
}

bool moduleIsModuleLoaded(const struct ts_module *p_module) {
    for(size_t l_i = 0; l_i < arrayListGetLength(&s_loadedModules); l_i++) {
        if(arrayListGet(&s_loadedModules, l_i) == p_module) {
            return true;
        }
    }

    return false;
}

static size_t moduleGetKernelModuleCount(void) {
    const size_t l_moduleSectionEnd = (size_t)&g_kernelModulesEnd;
    const size_t l_moduleSectionStart = (size_t)g_kernelModules;
    const size_t l_moduleSectionSize =
        l_moduleSectionEnd - l_moduleSectionStart;

    return l_moduleSectionSize / sizeof(struct ts_module);
}
