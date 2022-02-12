// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>

#include "arch/x86/mmu.h"
#include "arch/x86/multiboot.h"
#include "klibc/klibc.h"

// =============================================================================
// Private constants declaration
// =============================================================================
/**
 * @brief This constant represents the maximum Multiboot 2 information structure
 *        tag ID.
 */
#define C_MAX_TAG_ID 21

// =============================================================================
// Private types declaration
// =============================================================================
/**
 * @brief This structure represents the Multiboot 2 information structure.
 */
struct ts_multiboot2Information {
    uint32_t totalSize; ///< The total size of the structure in bytes.
    uint32_t reserved;  ///< A reserved field, set to 0.
    uint32_t data[];    ///< The structure data.
} __attribute__((packed));

/**
 * @brief This structure represents a Multiboot 2 tag.
 */
struct ts_multiboot2InformationTag {
    struct ts_multiboot2InformationTagHeader header; ///< The header of the tag.
    uint32_t data[];                                 ///< The tag data.
} __attribute__((packed));

// =============================================================================
// Private functions declaration
// =============================================================================
/**
 * @brief Maps the multiboot 2 information structure into memory.
 *
 * @param[in] p_multibootInfo A pointer to the physical address of the
 *                            multiboot information structure.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval 1 if the structure could not be mapped in memory.
 */
static int mapMultibootInfoStructure(const void *p_multibootInfo);

/**
 * @brief Parses the multiboot 2 information structure.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval 1 if the structure could not be parsed.
 */
static int parseMultibootInfoStructure(void);

// =============================================================================
// Private variables declaration
// =============================================================================
static const struct ts_multiboot2Information *s_multiboot2Information;
static uint32_t s_multibootInformationStructureSizeBytes;
static const struct ts_multiboot2InformationTag *s_tags[C_MAX_TAG_ID + 1];

// =============================================================================
// Public functions definition
// =============================================================================
int multibootInit(const void *p_multibootInfo) {
    kmemset(s_tags, 0, sizeof(s_tags));

    if(mapMultibootInfoStructure(p_multibootInfo) != 0) {
        return 2;
    } else if(parseMultibootInfoStructure()) {
        return 1;
    }

    return 0;
}

// =============================================================================
// Private functions definition
// =============================================================================
static int mapMultibootInfoStructure(const void *p_multibootInfo) {
    // Determine the number of pages to map for the header to be complete
    size_t l_multibootInfoStructureAddress = (size_t)p_multibootInfo;
    size_t l_structurePageOffset = l_multibootInfoStructureAddress & 0x00000fff;
    int l_nbPagesToMap = 1;

    if(l_structurePageOffset > 0x00000ffc) {
        l_nbPagesToMap = 2;
    }

    const void *l_mappedPages = mmuMapFrames(
        p_multibootInfo,
        l_nbPagesToMap,
        true
    );

    if(l_mappedPages == NULL) {
        return 1;
    }

    const struct ts_multiboot2Information *l_multibootInfoStructure =
        (const struct ts_multiboot2Information *)(
            ((size_t)l_mappedPages)
            + l_structurePageOffset
        );

    // Compute the number of pages to map
    s_multibootInformationStructureSizeBytes =
        l_multibootInfoStructure->totalSize;
    int l_nbPagesToFree = l_nbPagesToMap;

    l_nbPagesToMap =
        (
            s_multibootInformationStructureSizeBytes
            + l_structurePageOffset
            + 0x00000fff
        ) >> 12;

    // Unmap the partial multiboot info structure
    mmuFreePagesAt(l_multibootInfoStructure, l_nbPagesToFree);

    // Map the complete multiboot info structure
    s_multiboot2Information = (const struct ts_multiboot2Information *)
        mmuMapFrames(p_multibootInfo, l_nbPagesToMap, true);

    if(s_multiboot2Information == NULL) {
        return 1;
    }

    s_multiboot2Information =
        (const struct ts_multiboot2Information *)(
            ((size_t)s_multiboot2Information)
            + l_structurePageOffset
        );

    return 0;
}

static int parseMultibootInfoStructure(void) {
    uint32_t l_index = 0;
    bool l_finished = false;

    while(!l_finished) {
        const struct ts_multiboot2InformationTag *l_currentTag =
            (const struct ts_multiboot2InformationTag *)
            &s_multiboot2Information->data[l_index];

        if(l_currentTag->header.type == E_MULTIBOOT2_INFO_TAG_STOP) {
            l_finished = true;
        } else if(l_currentTag->header.type <= C_MAX_TAG_ID) {
            s_tags[l_currentTag->header.type] = l_currentTag;
        }

        l_index += (l_currentTag->header.size + 3) >> 2;

        // If the end of the tag was on an odd index, skip the next word.
        if((l_index & 0x00000001) != 0) {
            l_index++;
        }
    }

    return 0;
}

const struct ts_multiboot2InformationTag *multibootGetTag(
    enum te_multiboot2TagId p_tagId
) {
    if(p_tagId > C_MAX_TAG_ID) {
        return NULL;
    } else {
        return s_tags[p_tagId];
    }
}
