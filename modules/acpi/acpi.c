#include <kernel/debug.h>
#include <kernel/module.h>

int acpiInit(const char *p_args) {
    debug("acpi: Hello world!\n");

    return 0;
}

void acpiQuit(void) {

}

M_DECLARE_MODULE struct ts_module g_moduleAcpi = {
    .a_name = "acpi",
    .a_init = acpiInit,
    .a_quit = acpiQuit
};
