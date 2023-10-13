#ifndef __INCLUDE_KERNEL_MODULE_H__
#define __INCLUDE_KERNEL_MODULE_H__

#define M_MODULE __attribute__((section(".modules"))) __attribute__((unused))

#define M_DECLARE_MODULE(p_name, p_init, p_exit) \
    __attribute__((section(".modules"))) __attribute((unused)) \
    static struct ts_module _s_module = { \
        .m_name = p_name, \
        .m_init = p_init, \
        .m_exit = p_exit \
    }

#define M_THIS_MODULE (&_s_module)

struct ts_module {
    const char *m_name;
    int (*m_init)(void);
    void (*m_exit)(void);
    void *a_reserved; // TODO: fix in-section padding problem and remove this
};

int modulesInit(void);
const struct ts_module *moduleGetKernelModule(const char *p_name);
int moduleInit(const struct ts_module *p_module);
void moduleExit(const struct ts_module *p_module);

#endif
