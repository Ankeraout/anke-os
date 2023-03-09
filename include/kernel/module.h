#ifndef __INCLUDE_KERNEL_MODULE_H__
#define __INCLUDE_KERNEL_MODULE_H__

#include <stdbool.h>
#include <stddef.h>

#define M_DECLARE_MODULE __attribute__((section(".modules")))

struct ts_module {
    const char *a_name;
    int (*a_init)(const char *p_args);
    void (*a_quit)(void);
};

/**
 * @brief Initializes the module subsystem.
 *
 * @retval 0 if the module subsystem was initialized successfully.
 * @retval Any other value if the module subsystem could not be initialized.
*/
int moduleInit(void);

/**
 * @brief Gets a module that was compiled with the kernel.
 *
 * @param[in] p_name The name of the module.
 *
 * @returns The requested kernel module.
 * @retval NULL if the requested module could not be found.
 */
const struct ts_module *moduleGetKernelModule(const char *p_name);

/**
 * @brief Gets a loaded module.
 *
 * @param[in] p_name The name of the module.
 *
 * @returns The requested loaded module.
 * @retval NULL if the requested loaded module could not be found.
 */
const struct ts_module *moduleGetLoadedModule(const char *p_name);

/**
 * @brief Returns true if the given module is loaded, false otherwise.
 *
 * @param[in] p_module The module.
 *
 * @retval An integer that indicates the result of the operation.
 * @retval true if the module is loaded.
 * @retval false if the module is not loaded.
 */
bool moduleIsModuleLoaded(const struct ts_module *p_module);

/**
 * @brief Loads the given module.
 *
 * @param[in] p_args The command-line passed to the module.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the module was initialized successfully.
 * @retval Any other value if an error occurred.
*/
int moduleLoad(const struct ts_module *p_module, const char *p_args);

/**
 * @brief Unloads the given module.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the module was unloaded successfully.
 * @retval Any other value if an error occurred.
 */
int moduleUnload(const struct ts_module *p_module);

#endif
