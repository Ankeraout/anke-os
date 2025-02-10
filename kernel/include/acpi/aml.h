#ifndef __INCLUDE_ACPI_NAMESPACE_H__
#define __INCLUDE_ACPI_NAMESPACE_H__

#include <stddef.h>
#include <stdint.h>

#include "acpi/acpi.h"
#include "acpi/dsdt.h"

enum te_acpiObjectType {
    E_ACPI_OBJECT_TYPE_SCOPE,
    E_ACPI_OBJECT_TYPE_DEVICE,
    E_ACPI_OBJECT_TYPE_NAME,
    E_ACPI_OBJECT_TYPE_METHOD
};

struct ts_acpiObjectHeader {
    enum te_acpiObjectType m_type;
};

struct ts_acpiNamedObjectHeader {
    enum te_acpiObjectType m_type;
    char m_name[4];
    struct ts_acpiNamedObjectHeader *m_parent;
    struct ts_acpiNamedObjectHeader *m_next;
};

struct ts_acpiScopeObject {
    struct ts_acpiNamedObjectHeader m_header;
    struct ts_acpiNamedObjectHeader *m_child;
};

int acpiParseSdt(struct ts_acpi *p_acpi, const struct ts_acpiDsdt *p_sdt);

#endif
