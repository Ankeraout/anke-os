#ifndef __KERNEL_ACPI_H__
#define __KERNEL_ACPI_H__

#include <stdbool.h>

extern const bool acpi_supported;
extern const bool acpi_initialized;
extern const int acpi_version_major;
extern const int acpi_version_minor;

/* Initializes ACPI on the system. */
void acpi_init();

#endif
