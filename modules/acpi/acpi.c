#include <string.h>

#include <kernel/debug.h>
#include <kernel/module.h>

static int acpiCall(int l_operation, void *p_arg);

int acpiInit(const char *p_args) {
    debug("acpi: Hello world!\n");

    return 0;
}

void acpiQuit(void) {

}

static int acpiCall(int l_operation, void *p_arg) {
    switch(l_operation) {
        case E_MODULECALL_INIT: return acpiInit((const char *)p_arg);
        case E_MODULECALL_QUIT:
            acpiQuit();
            return E_MODULECALLRESULT_SUCCESS;

        default:
            return E_MODULECALLRESULT_NOT_IMPLEMENTED;
    }
}

M_DECLARE_MODULE struct ts_module g_moduleAcpi = {
    .a_name = "acpi",
    .a_call = acpiCall
};
