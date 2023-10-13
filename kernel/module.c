#include "kernel/module.h"
#include "klibc/debug.h"
#include "klibc/string.h"
#include "util/list.h"

extern const struct ts_module g_kernelModules[];
static struct ts_list *s_initializedModules;

int modulesInit(void) {
    s_initializedModules = NULL;

    return 0;
}

const struct ts_module *moduleGetKernelModule(const char *p_name) {
    extern void *g_kernelModulesEnd;
    const size_t l_moduleSectionSize =
        ((size_t)&g_kernelModulesEnd) - ((size_t)g_kernelModules);

    size_t l_moduleCount = l_moduleSectionSize / sizeof(struct ts_module);

    for(size_t l_i = 0; l_i < l_moduleCount; l_i++) {
        if(kstrcmp(g_kernelModules[l_i].m_name, p_name) == 0) {
            return &g_kernelModules[l_i];
        }
    }

    return NULL;
}

int moduleInit(const struct ts_module *p_module) {
    if(listContains(s_initializedModules, (void *)p_module)) {
        return -1;
    }

    int l_returnValue = listAdd(&s_initializedModules, (void *)p_module);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    l_returnValue = p_module->m_init();

    if(l_returnValue != 0) {
        listRemove(&s_initializedModules, (void *)p_module);
        return l_returnValue;
    }

    return 0;
}

void moduleExit(const struct ts_module *p_module) {
    p_module->m_exit();
    listRemove(&s_initializedModules, (void *)p_module);
}