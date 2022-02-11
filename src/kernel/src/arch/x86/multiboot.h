#ifndef __INC_ARCH_X86_MULTIBOOT_H__
#define __INC_ARCH_X86_MULTIBOOT_H__

// =============================================================================
// Public constants declaration
// =============================================================================
/**
 * @brief This enum represents a Multiboot 2 information structure tag ID.
 */
enum te_multiboot2TagId {
    E_MULTIBOOT2_INFO_TAG_STOP = 0, /**< This constant defines the "stop" tag
                                         ID. */
    E_MULTIBOOT2_INFO_TAG_FRAMEBUFFER = 8 /**< This constant defines the
                                               "framebuffer" tag ID. */
};

// =============================================================================
// Public types declaration
// =============================================================================
/**
 * @brief This structure represents the Multiboot 2 tag header.
 */
struct ts_multiboot2InformationTagHeader {
    uint32_t type; ///< The type of the tag.
    uint32_t size; ///< The size of the tag in bytes.
} __attribute__((packed));

/**
 * @brief This structure represents the Multiboot 2 framebuffer tag color info
 *        data for one primitive color (red, green or blue).
 */
struct ts_multiboot2InformationTagFramebufferDirectColor {
    uint8_t fieldPosition; ///< The position of the field (shift).
    uint8_t maskSize;      ///< The size of the mask (bit length).
} __attribute__((packed));

/**
 * @brief This structure represents the Multiboot 2 framebuffer tag color info
 *        data for a video mode with palette.
 */
struct ts_multiboot2InformationTagFramebufferColorInfoPalette {
    uint32_t nbColors; ///< The number of colors in the palette.

    struct {
        uint8_t red;   ///< The red component of the color.
        uint8_t green; ///< The green component of the color.
        uint8_t blue;  ///< The blue component of the color.
    } __attribute__((packed)) palette[];
} __attribute__((packed));

/**
 * @brief This structure represents the Multiboot 2 framebuffer tag color info
 *        data for all primitive colors.
 */
struct ts_multiboot2InformationTagFramebufferColorInfoDirect {
    struct ts_multiboot2InformationTagFramebufferDirectColor red;
        ///< Information for the red color component.

    struct ts_multiboot2InformationTagFramebufferDirectColor green;
        ///< Information for the green color component.

    struct ts_multiboot2InformationTagFramebufferDirectColor blue;
        ///< Information for the blue color component.
} __attribute__((packed));

/**
 * @brief This structure represents the Multiboot 2 framebuffer tag.
 */
struct ts_multiboot2InformationTagFramebuffer {
    struct ts_multiboot2InformationTagHeader header; ///< The header of the tag.
    uint64_t framebufferAddress; ///< The address of the framebuffer.
    uint32_t framebufferPitch;   /**< The pitch (number of bytes per line) of
                                      the framebuffer. */
    uint32_t framebufferWidth;   ///< The number of pixels in a row.
    uint32_t framebufferHeight;  ///< The number of rows.
    uint8_t framebufferBpp;      ///< The number of color bits per pixel.
    uint8_t framebufferType;     ///< The type of the framebuffer.
    uint8_t reserved;            ///< Reserved bits.
    uint8_t colorInfo[];         ///< Color information data.
} __attribute__((packed));

// =============================================================================
// Public functions declaration
// =============================================================================
/**
 * @brief Initializes the multiboot module by reading the multiboot information
 *        structure.
 *
 * @param[in] p_multibootInfo A pointer to the physical address of the multiboot
 *                            information structure.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the multiboot initialization was successful.
 * @retval 1 if the parameter points to invalid data.
 * @retval 2 if the data could not be read.
 */
int multibootInit(const void *p_multibootInfo);

/**
 * @brief Gets the requested Multiboot information structure tag.
 * 
 * @param[in] p_tagId The ID of the tag to get.
 * 
 * @returns A pointer to the tag.
 * @retval NULL if the tag was not found in the Multiboot 2 information
 *         structure.
 */
const struct ts_multiboot2InformationTag *multibootGetTag(
    enum te_multiboot2TagId p_tagId
);

#endif // __INC_ARCH_X86_MULTIBOOT_H__
