#include "acpi/aml.h"
#include "printk.h"
#include "string.h"

#define C_ACPI_MAX_NAME_LENGTH 256

enum te_acpiAmlOpcode {
    E_ACPI_AML_OPCODE_ZERO_OP = 0x00,
    E_ACPI_AML_OPCODE_ONE_OP = 0x01,
    E_ACPI_AML_OPCODE_ALIAS_OP = 0x06,
    E_ACPI_AML_OPCODE_NAME_OP = 0x08,
    E_ACPI_AML_OPCODE_BYTE_PREFIX = 0x0A,
    E_ACPI_AML_OPCODE_WORD_PREFIX = 0x0B,
    E_ACPI_AML_OPCODE_DWORD_PREFIX = 0x0C,
    E_ACPI_AML_OPCODE_STRING_PREFIX = 0x0D,
    E_ACPI_AML_OPCODE_QWORD_PREFIX = 0x0E,
    E_ACPI_AML_OPCODE_SCOPE_OP = 0x10,
    E_ACPI_AML_OPCODE_BUFFER_OP = 0x11,
    E_ACPI_AML_OPCODE_PACKAGE_OP = 0x12,
    E_ACPI_AML_OPCODE_VAR_PACKAGE_OP = 0x13,
    E_ACPI_AML_OPCODE_METHOD_OP = 0x14,
    E_ACPI_AML_OPCODE_EXTERNAL_OP = 0x15,
    E_ACPI_AML_OPCODE_DUAL_NAME_PREFIX = 0x2E,
    E_ACPI_AML_OPCODE_MULTI_NAME_PREFIX = 0x2F,
    E_ACPI_AML_OPCODE_EXT_OP_PREFIX = 0x5B,
    E_ACPI_AML_OPCODE_EXT_MUTEX_OP = 0x01,
    E_ACPI_AML_OPCODE_EXT_EVENT_OP = 0x02,
    E_ACPI_AML_OPCODE_EXT_COND_REF_OF_OP = 0x12,
    E_ACPI_AML_OPCODE_EXT_CREATE_FIELD_OP = 0x13,
    E_ACPI_AML_OPCODE_EXT_LOAD_TABLE_OP = 0x1F,
    E_ACPI_AML_OPCODE_EXT_LOAD_OP = 0x20,
    E_ACPI_AML_OPCODE_EXT_STALL_OP = 0x21,
    E_ACPI_AML_OPCODE_EXT_SLEEP_OP = 0x22,
    E_ACPI_AML_OPCODE_EXT_ACQUIRE_OP = 0x23,
    E_ACPI_AML_OPCODE_EXT_SIGNAL_OP = 0x24,
    E_ACPI_AML_OPCODE_EXT_WAIT_OP = 0x25,
    E_ACPI_AML_OPCODE_EXT_RESET_OP = 0x26,
    E_ACPI_AML_OPCODE_EXT_RELEASE_OP = 0x27,
    E_ACPI_AML_OPCODE_EXT_FROM_BCD_OP = 0x28,
    E_ACPI_AML_OPCODE_EXT_TO_BCD_OP = 0x29,
    E_ACPI_AML_OPCODE_EXT_REVISION_OP = 0x30,
    E_ACPI_AML_OPCODE_EXT_DEBUG_OP = 0x31,
    E_ACPI_AML_OPCODE_EXT_FATAL_OP = 0x32,
    E_ACPI_AML_OPCODE_EXT_TIMER_OP = 0x33,
    E_ACPI_AML_OPCODE_EXT_OP_REGION_OP = 0x80,
    E_ACPI_AML_OPCODE_EXT_FIELD_OP = 0x81,
    E_ACPI_AML_OPCODE_EXT_DEVICE_OP = 0x82,
    E_ACPI_AML_OPCODE_EXT_POWER_RES_OP = 0x84,
    E_ACPI_AML_OPCODE_EXT_THERMAL_ZONE_OP = 0x85,
    E_ACPI_AML_OPCODE_EXT_INDEX_FIELD_OP = 0x86,
    E_ACPI_AML_OPCODE_EXT_BANK_FIELD_OP = 0x87,
    E_ACPI_AML_OPCODE_EXT_DATA_REGION_OP = 0x88,
    E_ACPI_AML_OPCODE_ROOT_CHAR = 0x5C,
    E_ACPI_AML_OPCODE_PARENT_PREFIX_CHAR = 0x5E,
    E_ACPI_AML_OPCODE_NAME_CHAR = 0x5F,
    E_ACPI_AML_OPCODE_LOCAL_0_OP = 0x60,
    E_ACPI_AML_OPCODE_LOCAL_1_OP = 0x61,
    E_ACPI_AML_OPCODE_LOCAL_2_OP = 0x62,
    E_ACPI_AML_OPCODE_LOCAL_3_OP = 0x63,
    E_ACPI_AML_OPCODE_LOCAL_4_OP = 0x64,
    E_ACPI_AML_OPCODE_LOCAL_5_OP = 0x65,
    E_ACPI_AML_OPCODE_LOCAL_6_OP = 0x66,
    E_ACPI_AML_OPCODE_LOCAL_7_OP = 0x67,
    E_ACPI_AML_OPCODE_ARG_0_OP = 0x68,
    E_ACPI_AML_OPCODE_ARG_1_OP = 0x69,
    E_ACPI_AML_OPCODE_ARG_2_OP = 0x6A,
    E_ACPI_AML_OPCODE_ARG_3_OP = 0x6B,
    E_ACPI_AML_OPCODE_ARG_4_OP = 0x6C,
    E_ACPI_AML_OPCODE_ARG_5_OP = 0x6D,
    E_ACPI_AML_OPCODE_ARG_6_OP = 0x6E,
    E_ACPI_AML_OPCODE_STORE_OP = 0x70,
    E_ACPI_AML_OPCODE_REF_OF_OP = 0x71,
    E_ACPI_AML_OPCODE_ADD_OP = 0x72,
    E_ACPI_AML_OPCODE_CONCAT_OP = 0x73,
    E_ACPI_AML_OPCODE_SUBTRACT_OP = 0x74,
    E_ACPI_AML_OPCODE_INCREMENT_OP = 0x75,
    E_ACPI_AML_OPCODE_DECREMENT_OP = 0x76,
    E_ACPI_AML_OPCODE_MULTIPLY_OP = 0x77,
    E_ACPI_AML_OPCODE_DIVIDE_OP = 0x78,
    E_ACPI_AML_OPCODE_SHIFT_LEFT_OP = 0x79,
    E_ACPI_AML_OPCODE_SHIFT_RIGHT_OP = 0x7A,
    E_ACPI_AML_OPCODE_AND_OP = 0x7B,
    E_ACPI_AML_OPCODE_NAND_OP = 0x7C,
    E_ACPI_AML_OPCODE_OR_OP = 0x7D,
    E_ACPI_AML_OPCODE_NOR_OP = 0x7E,
    E_ACPI_AML_OPCODE_XOR_OP = 0x7F,
    E_ACPI_AML_OPCODE_NOT_OP = 0x80,
    E_ACPI_AML_OPCODE_FIND_SET_LEFT_BIT_OP = 0x81,
    E_ACPI_AML_OPCODE_FIND_SET_RIGHT_BIT_OP = 0x82,
    E_ACPI_AML_OPCODE_DEREF_OF_OP = 0x83,
    E_ACPI_AML_OPCODE_CONCAT_RES_OP = 0x84,
    E_ACPI_AML_OPCODE_MOD_OP = 0x85,
    E_ACPI_AML_OPCODE_NOTIFY_OP = 0x86,
    E_ACPI_AML_OPCODE_SIZE_OF_OP = 0x87,
    E_ACPI_AML_OPCODE_INDEX_OP = 0x88,
    E_ACPI_AML_OPCODE_MATCH_OP = 0x89,
    E_ACPI_AML_OPCODE_CREATE_DWORD_FIELD_OP = 0x8A,
    E_ACPI_AML_OPCODE_CREATE_WORD_FIELD_OP = 0x8B,
    E_ACPI_AML_OPCODE_CREATE_BYTE_FIELD_OP = 0x8C,
    E_ACPI_AML_OPCODE_CREATE_BIT_FIELD_OP = 0x8D,
    E_ACPI_AML_OPCODE_OBJECT_TYPE_OP = 0x8E,
    E_ACPI_AML_OPCODE_CREATE_QWORD_FIELD_OP = 0x8F,
    E_ACPI_AML_OPCODE_LAND_OP = 0x90,
    E_ACPI_AML_OPCODE_LOR_OP = 0x91,
    E_ACPI_AML_OPCODE_LNOT_OP = 0x92,
    E_ACPI_AML_OPCODE_LEQUAL_OP = 0x93,
    E_ACPI_AML_OPCODE_LGREATER_OP = 0x94,
    E_ACPI_AML_OPCODE_LLESS_OP = 0x95,
    E_ACPI_AML_OPCODE_TO_BUFFER_OP = 0x96,
    E_ACPI_AML_OPCODE_TO_DEC_STRING_OP = 0x97,
    E_ACPI_AML_OPCODE_TO_HEX_STRING_OP = 0x98,
    E_ACPI_AML_OPCODE_TO_INTEGER_OP = 0x99,
    E_ACPI_AML_OPCODE_TO_STRING_OP = 0x9C,
    E_ACPI_AML_OPCODE_COPY_OBJECT_OP = 0x9D,
    E_ACPI_AML_OPCODE_MID_OP = 0x9E,
    E_ACPI_AML_OPCODE_CONTINUE_OP = 0x9F,
    E_ACPI_AML_OPCODE_IF_OP = 0xA0,
    E_ACPI_AML_OPCODE_ELSE_OP = 0xA1,
    E_ACPI_AML_OPCODE_WHILE_OP = 0xA2,
    E_ACPI_AML_OPCODE_NO_OP = 0xA3,
    E_ACPI_AML_OPCODE_RETURN_OP = 0xA4,
    E_ACPI_AML_OPCODE_BREAK_OP = 0xA5,
    E_ACPI_AML_OPCODE_BREAK_POINT_OP = 0xCC,
    E_ACPI_AML_OPCODE_ONES_OP = 0xFF
};

struct ts_acpiAmlParseContext {
    struct ts_acpi *m_acpi;
    const uint8_t *m_data;
    size_t m_size;
    size_t m_offset;
};

static int acpiAmlInitParseContext(
    struct ts_acpiAmlParseContext *p_context,
    struct ts_acpi *p_acpi,
    const struct ts_acpiDsdt *p_dsdt
);
static int acpiAmlParseTermList(
    struct ts_acpiAmlParseContext *p_context,
    size_t p_size
);
static int acpiAmlParseTermObj(struct ts_acpiAmlParseContext *p_context);
static int acpiAmlParsePkgLength(
    struct ts_acpiAmlParseContext *p_context,
    size_t *p_length
);
static int acpiAmlParseNameString(
    struct ts_acpiAmlParseContext *p_context,
    char *p_name
);
static int acpiAmlParseNameSeg(
    struct ts_acpiAmlParseContext *p_context,
    char *p_name,
    size_t *p_length
);
static int acpiAmlParseDefScope(struct ts_acpiAmlParseContext *p_context);
static int acpiAmlParseDefName(struct ts_acpiAmlParseContext *p_context);
static int acpiAmlParseDefPackage(struct ts_acpiAmlParseContext *p_context);

int acpiParseSdt(struct ts_acpi *p_acpi, const struct ts_acpiDsdt *p_sdt) {
    struct ts_acpiAmlParseContext l_context;
    const size_t l_size =
        p_sdt->m_header.m_length - sizeof(struct ts_acpiSdtHeader);

    int l_result = acpiAmlInitParseContext(&l_context, p_acpi, p_sdt);

    if(l_result != 0) {
        return l_result;
    }

    return acpiAmlParseTermList(&l_context, l_size);
}

static int acpiAmlInitParseContext(
    struct ts_acpiAmlParseContext *p_context,
    struct ts_acpi *p_acpi,
    const struct ts_acpiDsdt *p_sdt
) {
    p_context->m_acpi = p_acpi;
    p_context->m_data = p_sdt->m_data;
    p_context->m_size =
        p_sdt->m_header.m_length - sizeof(struct ts_acpiSdtHeader);
    p_context->m_offset = 0;

    return 0;
}

static int acpiAmlParseTermList(
    struct ts_acpiAmlParseContext *p_context,
    size_t p_size
) {
    size_t l_endOffset = p_context->m_offset + p_size;

    while(p_context->m_offset < l_endOffset) {
        int result = acpiAmlParseTermObj(p_context);

        if(result != 0) {
            return result;
        }
    }

    return 0;
}

static int acpiAmlParseTermObj(struct ts_acpiAmlParseContext *p_context) {
    const uint8_t l_opcode = p_context->m_data[p_context->m_offset++];

    pr_info("acpi: Parsing term obj at 0x%04x\n", p_context->m_offset);

    switch(l_opcode) {
        /*
        case E_ACPI_AML_OPCODE_ALIAS_OP:
            return acpiAmlParseDefAlias(p_context);
        */

        case E_ACPI_AML_OPCODE_NAME_OP:
            return acpiAmlParseDefName(p_context);

        case E_ACPI_AML_OPCODE_SCOPE_OP:
            return acpiAmlParseDefScope(p_context);

        case E_ACPI_AML_OPCODE_PACKAGE_OP:
            return acpiAmlParseDefPackage(p_context);

        default:
            pr_err("acpi: Unexpected AML opcode: 0x%02x\n", l_opcode);
            return -1;
    }
}

static int acpiAmlParsePkgLength(
    struct ts_acpiAmlParseContext *p_context,
    size_t *p_length
) {
    const uint8_t l_pkgLeadByte = p_context->m_data[p_context->m_offset];
    const uint8_t l_byteCount = (l_pkgLeadByte >> 6U) + 1U;

    if(l_byteCount == 1U) {
        *p_length = l_pkgLeadByte & 0x3fU;
    } else {
        size_t l_pkgLength = l_pkgLeadByte & 0x0fU;

        for(size_t l_offset = 1; l_offset < l_byteCount; l_offset++) {
            l_pkgLength |= (
                p_context->m_data[p_context->m_offset + l_offset]
                << (8U * (l_offset - 1U) + 4U)
            );
        }

        *p_length = l_pkgLength;
    }

    p_context->m_offset += l_byteCount;

    return 0;
}

static int acpiAmlParseNameString(
    struct ts_acpiAmlParseContext *p_context,
    char *p_name
) {
    size_t l_length = 0;
    size_t l_segmentLength;
    size_t l_segmentCount;

    while(true) {
        switch(p_context->m_data[p_context->m_offset]) {
            case 0: // Null name
                p_context->m_offset++;
                p_name[l_length] = '\0';
                break;

            case 0x2e: // Dual name prefix
                p_context->m_offset++;

                for(size_t l_index = 0; l_index < 2; l_index++) {
                    if(l_index > 0) {
                        p_name[l_length++] = '.';
                    }

                    acpiAmlParseNameSeg(
                        p_context,
                        &p_name[l_length],
                        &l_segmentLength
                    );

                    l_length += l_segmentLength;
                }

                break;

            case 0x2f: // Multi name prefix
                p_context->m_offset++;
                l_segmentCount = p_context->m_data[p_context->m_offset++];

                for(size_t l_index = 0; l_index < l_segmentCount; l_index++) {
                    if(l_index > 0) {
                        p_name[l_length++] = '.';
                    }

                    acpiAmlParseNameSeg(
                        p_context,
                        &p_name[l_length],
                        &l_segmentLength
                    );

                    l_length += l_segmentLength;
                }
                
                break;
            
            case '\\':
            case '^':
                p_name[l_length++] = p_context->m_data[p_context->m_offset];
                p_context->m_offset++;
                continue;

            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
            case '_':
                acpiAmlParseNameSeg(
                    p_context,
                    &p_name[l_length],
                    &l_segmentLength
                );
                
                l_length += l_segmentLength;

                break;

            default:
                pr_err(
                    "acpi: Unexpected character in name string: 0x%02x\n",
                    p_context->m_data[p_context->m_offset]
                );

                return -1;
        }

        break;
    }

    p_name[l_length] = '\0';

    return 0;
}

static int acpiAmlParseNameSeg(
    struct ts_acpiAmlParseContext *p_context,
    char *p_name,
    size_t *p_length
) {
    bool l_isSuffix = true;
    size_t l_length = 0;

    for(int l_offset = 3; l_offset >= 0; l_offset--) {
        if(
            l_isSuffix
            && (p_context->m_data[p_context->m_offset + l_offset] != '_')
        ) {
            l_isSuffix = false;
            l_length = l_offset + 1;
        }

        if(!l_isSuffix) {
            p_name[l_offset] =
                p_context->m_data[p_context->m_offset + l_offset];
        }
    }

    p_context->m_offset += 4;
    *p_length = l_length;

    return 0;
}

static int acpiAmlParseDefScope(struct ts_acpiAmlParseContext *p_context) {
    const size_t l_pkgStartOffset = p_context->m_offset;
    size_t l_pkgLength;

    int l_result = acpiAmlParsePkgLength(p_context, &l_pkgLength);

    if(l_result != 0) {
        return l_result;
    }

    const size_t l_pkgLengthLength = p_context->m_offset - l_pkgStartOffset;

    char l_name[C_ACPI_MAX_NAME_LENGTH];

    l_result = acpiAmlParseNameString(p_context, l_name);

    if(l_result != 0) {
        return l_result;
    }

    const size_t l_nameLength =
        p_context->m_offset - l_pkgStartOffset - l_pkgLengthLength;

    pr_info("acpi: DefScope: %s (size: %llu)\n", l_name, l_pkgLength);

    //return acpiAmlParseTermList(p_context, l_pkgLength - l_pkgLengthLength);

    p_context->m_offset += l_pkgLength - l_pkgLengthLength - l_nameLength;

    return 0;
}

static int acpiAmlParseDefName(struct ts_acpiAmlParseContext *p_context) {
    char l_name[C_ACPI_MAX_NAME_LENGTH];

    int l_result = acpiAmlParseNameString(p_context, l_name);

    if(l_result != 0) {
        return l_result;
    }

    pr_info("acpi: DefName: %s\n", l_name);

    l_result = acpiAmlParseTermObj(p_context);

    if(l_result != 0) {
        return l_result;
    }

    // TODO: store term object

    return 0;
}

static int acpiAmlParseDefPackage(struct ts_acpiAmlParseContext *p_context) {
    size_t l_pkgLength;
    int l_result = acpiAmlParsePkgLength(p_context, &l_pkgLength);

    if(l_result != 0) {
        return l_result;
    }

    uint8_t l_numElements = p_context->m_data[p_context->m_offset++];

    
}
