#ifndef __INCLUDE_ACPI_NAMESPACE_H__
#define __INCLUDE_ACPI_NAMESPACE_H__

#include <stddef.h>
#include <stdint.h>

#include "acpi/acpi.h"
#include "acpi/dsdt.h"

#define C_ACPI_NAME_LENGTH 4

enum te_acpi_objectType {
    E_ACPI_OBJECT_TYPE_SCOPE,
    E_ACPI_OBJECT_TYPE_DEVICE,
    E_ACPI_OBJECT_TYPE_NAME,
    E_ACPI_OBJECT_TYPE_METHOD,
    E_ACPI_OBJECT_TYPE_PROCESSOR,
    E_ACPI_OBJECT_TYPE_THERMAL_ZONE,
    E_ACPI_OBJECT_TYPE_POWER_RESOURCE,
    E_ACPI_OBJECT_TYPE_EVENT,
    E_ACPI_OBJECT_TYPE_OPERATION_REGION,
    E_ACPI_OBJECT_TYPE_FIELD,
    E_ACPI_OBJECT_TYPE_ALIAS,
    E_ACPI_OBJECT_TYPE_MUTEX,
    E_ACPI_OBJECT_TYPE_SEMAPHORE
};

enum te_acpi_dataObjectType {
    E_ACPI_DATA_OBJECT_TYPE_INTEGER,
    E_ACPI_DATA_OBJECT_TYPE_STRING,
    E_ACPI_DATA_OBJECT_TYPE_BUFFER,
    E_ACPI_DATA_OBJECT_TYPE_PACKAGE,
    E_ACPI_DATA_OBJECT_TYPE_REFERENCE
};

struct ts_acpi_node {
    char m_name[C_ACPI_NAME_LENGTH + 1];
    enum te_acpi_objectType m_type;
    struct ts_acpi_node *m_parent;
    struct ts_acpi_node *m_next;
    struct ts_acpi_node *m_children;
};

int acpi_parseAml(struct ts_acpi *p_acpi, const struct ts_acpi_sdt *p_sdt);

#endif
