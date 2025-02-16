#include "acpi/aml.h"
#include "printk.h"
#include "stdlib.h"
#include "string.h"

#define C_ACPI_MAX_PATH_LENGTH 256

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

/**
 * @brief Parses an AML scope definition.
 * 
 * @param[in] p_node A pointer to the node.
 * @param[in] p_buffer A pointer to the buffer.
 * @param[out] p_length The length of the buffer.
 * 
 * @returns 0 on success, -1 on failure.
 */
static int acpiAmlParseDefScope(
    struct ts_acpiNode *p_node,
    const uint8_t *p_buffer,
    size_t *p_length
);

/**
 * @brief Parses a name segment from an AML buffer.
 * 
 * @param[in] p_buffer A pointer to the buffer.
 * @param[out] p_name A pointer to the name buffer.
 * @param[out] p_length The length of the name segment.
 * 
 * @returns 0 on success, -1 on failure.
 */
static int acpiAmlParseNameSeg(
    const uint8_t *p_buffer,
    char *p_name,
    size_t *p_length
);

/**
 * @brief Parses a name string from an AML buffer.
 * 
 * @param[in] p_buffer A pointer to the buffer.
 * @param[out] p_name A pointer to the name buffer.
 * @param[out] p_length The length of the name string.
 * 
 * @returns 0 on success, -1 on failure.
 */
static int acpiAmlParseNameString(
    const uint8_t *p_buffer,
    char *p_name,
    size_t *p_length
);

/**
 * @brief Parses a package length from an AML buffer.
 * 
 * @param[in] p_buffer A pointer to the buffer.
 * @param[out] p_pkgLength The package length.
 * 
 * @returns The number of bytes read.
 */
static size_t acpiAmlParsePkgLength(
    const uint8_t *p_buffer,
    size_t *p_pkgLength
);

/**
 * @brief Parses a term list from an AML buffer.
 * 
 * @param[in] p_node A pointer to the node.
 * @param[in] p_buffer A pointer to the buffer.
 * @param[in] p_length The length of the buffer.
 * 
 * @returns 0 on success, -1 on failure.
 */
static int acpiAmlParseTermList(
    struct ts_acpiNode *p_node,
    const uint8_t *p_buffer,
    size_t p_length
);

/**
 * @brief Parses a term obj from an AML buffer.
 * 
 * @param[in] p_node A pointer to the node.
 * @param[in] p_buffer A pointer to the buffer.
 * @param[in] p_length The length of the parsed object.
 * 
 * @returns 0 on success, -1 on failure.
 */
static int acpiAmlParseTermObj(
    struct ts_acpiNode *p_node,
    const uint8_t *p_buffer,
    size_t *p_length
);

/**
 * @brief Creates a new node.
 * 
 * @param[in] p_parent The parent node.
 * @param[out] p_node A pointer to the node.
 * 
 * @returns 0 on success, -1 on failure.
 */
static int acpiCreateNode(
    struct ts_acpiNode *p_parent,
    struct ts_acpiNode **p_node
);

/**
 * @brief Gets a child node by name.
 * 
 * @param[in] p_node A pointer to the node.
 * @param[in] p_name The name.
 * @param[out] p_child A pointer to the child node.
 * 
 * @returns 0 on success, -1 on failure.
 */
static int acpiGetChildByName(
    struct ts_acpiNode *p_node,
    const char *p_name,
    struct ts_acpiNode **p_child
);

/**
 * @brief Gets a node by path.
 * 
 * @param[in] p_baseNode A pointer to the node to start the search from.
 * @param[in] p_path The path.
 * @param[out] p_node A pointer to the node.
 * 
 * @returns 0 on success, -1 on failure.
 */
static int acpiGetNode(
    struct ts_acpiNode *p_baseNode,
    const char *p_path,
    struct ts_acpiNode **p_node
);

/**
 * @brief Gets the name of a path.
 * 
 * @param[in] p_path The path.
 * @param[out] p_name A pointer to the name.
 */
static void acpiGetPathName(const char *p_path, char *p_name);

/**
 * @brief Gets a parent node by path.
 * 
 * @param[in] p_baseNode A pointer to the node to start the search from.
 * @param[in] p_path The path.
 * @param[out] p_node A pointer to the node.
 * 
 * @returns 0 on success, -1 on failure.
 */
static int acpiGetParentByPath(
    struct ts_acpiNode *p_baseNode,
    const char *p_path,
    struct ts_acpiNode **p_node
);

/**
 * @brief Gets the root node.
 * 
 * @param[in] p_baseNode A pointer to the base node.
 * 
 * @returns A pointer to the root node.
 */
static struct ts_acpiNode *acpiGetRootNode(struct ts_acpiNode *p_baseNode);

int acpiParseAml(struct ts_acpi *p_acpi, const struct ts_acpiDsdt *p_sdt) {
    // Initialize root node
    if(p_acpi->m_root == NULL) {
        int l_result = acpiCreateNode(NULL, &p_acpi->m_root);

        if(l_result != 0) {
            return l_result;
        }

        p_acpi->m_root->m_name[0] = '\\';
    }

    return acpiAmlParseTermList(
        p_acpi->m_root,
        p_sdt->m_data,
        p_sdt->m_header.m_length - sizeof(struct ts_acpiSdtHeader)
    );
}

static int acpiAmlParseDefScope(
    struct ts_acpiNode *p_node,
    const uint8_t *p_buffer,
    size_t *p_length
) {
    size_t l_offset = 0U;

    if(p_buffer[l_offset++] != E_ACPI_AML_OPCODE_SCOPE_OP) {
        pr_err("acpi: Expected scope opcode\n");
        return -1;
    }

    // Parse package length
    size_t l_pkgLength;
    l_offset += acpiAmlParsePkgLength(&p_buffer[l_offset], &l_pkgLength);

    // Parse name
    char l_name[C_ACPI_MAX_PATH_LENGTH];
    size_t l_nameLength;

    int l_result =
        acpiAmlParseNameString(&p_buffer[l_offset], l_name, &l_nameLength);

    if(l_result != 0) {
        return l_result;
    }

    l_offset += l_nameLength;

    // Get parent node
    struct ts_acpiNode *l_parentNode;
    l_result = acpiGetParentByPath(p_node, l_name, &l_parentNode);

    if(l_result != 0) {
        return l_result;
    }

    struct ts_acpiNode *l_node;
    
    // Get the existing node, and if it does not exist, create it.
    if(acpiGetChildByName(l_parentNode, l_name, &l_node) != 0) {
        l_result = acpiCreateNode(l_parentNode, &l_node);

        if(l_node == NULL) {
            return -1;
        }

        l_node->m_type = E_ACPI_OBJECT_TYPE_SCOPE;
        
        acpiGetPathName(l_name, l_node->m_name);
    }

    // Parse term list
    l_result = acpiAmlParseTermList(
        l_node,
        &p_buffer[l_offset],
        l_pkgLength - l_offset + 1
    );

    if(l_result != 0) {
        return l_result;
    }

    // Return the length of the parsed object
    *p_length = l_pkgLength + 1;

    return 0;
}

static int acpiAmlParseNameSeg(
    const uint8_t *p_buffer,
    char *p_name,
    size_t *p_length
) {
    bool l_isSuffix = true;
    size_t l_length = 0;

    for(int l_offset = 3; l_offset >= 0; l_offset--) {
        if(
            l_isSuffix
            && (p_buffer[l_offset] != '_')
        ) {
            l_isSuffix = false;
            l_length = l_offset + 1;
        }

        if(!l_isSuffix) {
            p_name[l_offset] = p_buffer[l_offset];
        }
    }

    // TODO: validate name

    *p_length = l_length;

    return 0;
}

static int acpiAmlParseNameString(
    const uint8_t *p_buffer,
    char *p_name,
    size_t *p_length
) {
    size_t l_length = 0;
    size_t l_segmentLength;
    size_t l_segmentCount;
    size_t l_offset = 0;

    while(true) {
        switch(p_buffer[l_offset]) {
            case 0: // Null name
                l_offset++;
                p_name[l_length] = '\0';
                break;

            case 0x2e: // Dual name prefix
                l_offset++;

                for(size_t l_index = 0; l_index < 2; l_index++) {
                    if(l_index > 0) {
                        p_name[l_length++] = '.';
                    }

                    if(
                        acpiAmlParseNameSeg(
                            &p_buffer[l_offset],
                            &p_name[l_length],
                            &l_segmentLength
                        ) != 0
                    ) {
                        return -1;
                    }

                    l_offset += 4U;

                    l_length += l_segmentLength;
                }

                break;

            case 0x2f: // Multi name prefix
                l_offset++;
                l_segmentCount = p_buffer[l_offset++];

                for(size_t l_index = 0; l_index < l_segmentCount; l_index++) {
                    if(l_index > 0) {
                        p_name[l_length++] = '.';
                    }

                    if(
                        acpiAmlParseNameSeg(
                            &p_buffer[l_offset],
                            &p_name[l_length],
                            &l_segmentLength
                        ) != 0
                    ) {
                        return -1;
                    }

                    l_offset += 4U;

                    l_length += l_segmentLength;
                }
                
                break;
            
            case '\\':
            case '^':
                p_name[l_length++] = p_buffer[l_offset];
                l_offset++;
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
                if(
                    acpiAmlParseNameSeg(
                        &p_buffer[l_offset],
                        &p_name[l_length],
                        &l_segmentLength
                    ) != 0
                ) {
                    return -1;
                }

                l_offset += 4U;
                
                l_length += l_segmentLength;

                break;

            default:
                pr_err(
                    "acpi: Unexpected character in name string: 0x%02x\n",
                    p_buffer[l_offset]
                );

                return -1;
        }

        break;
    }

    p_name[l_length] = '\0';

    *p_length = l_offset;

    return 0;
}

static size_t acpiAmlParsePkgLength(
    const uint8_t *p_buffer,
    size_t *p_pkgLength
) {
    const uint8_t l_pkgLeadByte = p_buffer[0];
    const uint8_t l_byteCount = (l_pkgLeadByte >> 6U) + 1U;

    if(l_byteCount == 1U) {
        *p_pkgLength = l_pkgLeadByte & 0x3fU;
    } else {
        size_t l_pkgLength = l_pkgLeadByte & 0x0fU;

        for(size_t l_offset = 1; l_offset < l_byteCount; l_offset++) {
            l_pkgLength |= (p_buffer[l_offset] << (8U * (l_offset - 1U) + 4U));
        }

        *p_pkgLength = l_pkgLength;
    }

    return l_byteCount;
}

static int acpiAmlParseTermList(
    struct ts_acpiNode *p_node,
    const uint8_t *p_buffer,
    size_t p_length
) {
    size_t l_offset = 0U;

    while(l_offset <= p_length) {
        size_t l_termObjectLength;
        
        int l_result = acpiAmlParseTermObj(
            p_node,
            &p_buffer[l_offset],
            &l_termObjectLength
        );

        if(l_result != 0) {
            return l_result;
        }

        l_offset += l_termObjectLength;
    }

    return 0;
}

static int acpiAmlParseTermObj(
    struct ts_acpiNode *p_node,
    const uint8_t *p_buffer,
    size_t *p_length
) {
    switch(p_buffer[0]) {
        case E_ACPI_AML_OPCODE_SCOPE_OP:
            return acpiAmlParseDefScope(p_node, p_buffer, p_length);
    }

    // Undefined opcode
    pr_err(
        "acpi: Undefined AML opcode: 0x%02x\n",
        p_buffer[0]
    );

    return -1;
}

static int acpiCreateNode(
    struct ts_acpiNode *p_parent,
    struct ts_acpiNode **p_node
) {
    struct ts_acpiNode *l_node = malloc(sizeof(struct ts_acpiNode));

    if (l_node == NULL) {
        printk("acpi: failed to allocate memory for node\n");
        return -1;
    }

    memset(l_node, 0, sizeof(struct ts_acpiNode));
    
    if(p_parent != NULL) {
        if(p_parent->m_children == NULL) {
            p_parent->m_children = l_node;
        } else {
            struct ts_acpiNode *l_child = p_parent->m_children;

            while(l_child->m_next != NULL) {
                l_child = l_child->m_next;
            }

            l_child->m_next = l_node;
        }

        l_node->m_parent = p_parent;
    }

    *p_node = l_node;

    return 0;
}

static int acpiGetChildByName(
    struct ts_acpiNode *p_node,
    const char *p_name,
    struct ts_acpiNode **p_child
) {
    struct ts_acpiNode *l_child = p_node->m_children;

    while(l_child != NULL) {
        if(strcmp(l_child->m_name, p_name) == 0) {
            *p_child = l_child;
            return 0;
        }

        l_child = l_child->m_next;
    }

    return -1;
}

static int acpiGetNode(
    struct ts_acpiNode *p_baseNode,
    const char *p_path,
    struct ts_acpiNode **p_node
) {
    struct ts_acpiNode *l_node = p_baseNode;

    if(p_path[0] == '\\') {
        p_path++;
        
        // Go back to the root node
        while(l_node->m_parent != NULL) {
            l_node = l_node->m_parent;
        }
    }

    while(*p_path != '\0') {
        if(*p_path == '.') {
            p_path++;
        }

        char l_name[C_ACPI_NAME_LENGTH + 1];
        int l_index = 0;

        while(*p_path != '.' && *p_path != '\0') {
            if(l_index >= C_ACPI_NAME_LENGTH) {
                printk("acpi: name too long\n");
                return -1;
            }

            l_name[l_index++] = *p_path++;
        }

        l_name[l_index] = '\0';

        struct ts_acpiNode *l_child = l_node->m_children;

        while(l_child != NULL) {
            if(strcmp(l_child->m_name, l_name) == 0) {
                l_node = l_child;
                break;
            }

            l_child = l_child->m_next;
        }

        if(l_child == NULL) {
            printk("acpi: node not found\n");
            return -1;
        }
    }

    *p_node = l_node;

    return 0;
}

static void acpiGetPathName(const char *p_path, char *p_name) {
    const char *l_lastDot = strrchr(p_path, '.');

    if(l_lastDot == NULL) {
        p_name[0] = '\0';
        return;
    }

    size_t l_nameLength = 0U;

    while(
        (l_nameLength < C_ACPI_NAME_LENGTH)
        && (l_lastDot[l_nameLength + 1] != '\0')
    ) {
        p_name[l_nameLength] = l_lastDot[l_nameLength + 1];
        l_nameLength++;
    }
    
    p_name[l_nameLength] = '\0';

    return;
}

static int acpiGetParentByPath(
    struct ts_acpiNode *p_baseNode,
    const char *p_path,
    struct ts_acpiNode **p_node
) {
    struct ts_acpiNode *l_node = p_baseNode;
    size_t l_pathIndex = 0U;
    char l_name[C_ACPI_NAME_LENGTH + 1];
    size_t l_nameIndex = 0U;
    const char *l_lastDot = strrchr(p_path, '.');

    if(l_lastDot == NULL) {
        *p_node = p_baseNode;
        return 0;
    }

    size_t l_lastDotIndex = l_lastDot - p_path;

    while(l_pathIndex <= l_lastDotIndex) {
        if(p_path[l_pathIndex] == '.') {
            l_name[l_nameIndex] = '\0';

            int l_result = acpiGetChildByName(l_node, l_name, &l_node);

            if(l_result != 0) {
                return l_result;
            }

            l_nameIndex = 0;
        }

        l_pathIndex++;
    }

    *p_node = l_node;

    return 0;
}

static struct ts_acpiNode *acpiGetRootNode(struct ts_acpiNode *p_baseNode) {
    struct ts_acpiNode *l_node = p_baseNode;

    while(l_node->m_parent != NULL) {
        l_node = l_node->m_parent;
    }

    return l_node;
}
