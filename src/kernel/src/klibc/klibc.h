#ifndef __INC_KLIBC_KLIBC_H__
#define __INC_KLIBC_KLIBC_H__

// =============================================================================
// File inclusion
// =============================================================================
#include <stdarg.h>
#include <stddef.h>

// =============================================================================
// Public functions declaration
// =============================================================================
/**
 * @brief Prints the given formatted text to the kernel standard output.
 *
 * @param[in] p_format The format of the output.
 *
 * @returns The number of characters in the formatted text.
 */
int kprintf(const char *p_format, ...);

/**
 * @brief Prints the given formatted text to the given buffer.
 *
 * @param[out] p_buffer The buffer where the formatted text shall be written.
 * @param[in] p_format The format of the output.
 *
 * @returns The number of characters in the formatted text.
 */
int ksprintf(char *p_buffer, const char *p_format, ...);

/**
 * @brief Prints the given formatted text to the given buffer, with a limit to
 *        the size of the buffer.
 *
 * @param[out] p_buffer The buffer where the formatted text shall be written.
 * @param[in] p_bufferSize The maximum number of characters that the buffer can
 *                         hold (that's the size of the buffer minus one).
 * @param[in] p_format The format of the output.
 *
 * @returns The number of characters in the formatted text.
 */
int ksnprintf(char *p_buffer, size_t p_bufferSize, const char *p_format, ...);
/**
 * @brief Prints the given formatted text to the given buffer, with a limit to
 *        the size of the buffer.
 *
 * @param[out] p_buffer The buffer where the formatted text shall be written.
 * @param[in] p_bufferSize The maximum number of characters that the buffer can
 *                         hold (that's the size of the buffer minus one).
 * @param[in] p_format The format of the output.
 * @param[in] p_argumentList The list of arguments for the formatted text.
 *
 * @returns The number of characters in the formatted text.
 */
int kvsnprintf(
    char *p_buffer,
    size_t p_bufferSize,
    const char *p_format,
    va_list p_argumentList
);

/**
 * @brief Prints the given formatted text to the given buffer.
 *
 * @param[out] p_buffer The buffer where the formatted text shall be written.
 * @param[in] p_format The format of the output.
 * @param[in] p_argumentList The list of arguments for the formatted text.
 *
 * @returns The number of characters in the formatted text.
 */
int kvsprintf(char *p_buffer, const char *p_format, va_list p_argumentList);

/**
 * @brief Prints the given formatted text to the kernel standard output.
 *
 * @param[in] p_format The format of the output.
 * @returns The number of characters in the formatted text.
 * @param[in] p_argumentList The list of arguments for the formatted text.
 *
 * @returns The number of characters in the formatted text.
 */
int kvprintf(const char *p_format, va_list p_argumentList);

/**
 * @brief Copies p_size bytes from p_source to p_destination in memory.
 *
 * @param[out] p_destination The destination buffer.
 * @param[in] p_source The source buffer.
 * @param[in] p_size The number of bytes to copy.
 *
 * @returns A pointer to the destination buffer (p_destination).
 */
void *kmemcpy(void *p_destination, const void *p_source, size_t p_size);

/**
 * @brief Copies p_size bytes from p_source to p_destination in memory. Unlike
 *        kmemcpy(), this function allows for the source and the destination
 *        buffers to overlap.
 *
 * @param[out] p_destination The destination buffer.
 * @param[in] p_source The source buffer.
 * @param[in] p_size The number of bytes to copy.
 *
 * @returns A pointer to the destination buffer (p_destination).
 */
void *kmemmove(void *p_destination, const void *p_source, size_t p_size);

/**
 * @brief Sets p_size bytes to p_value in p_buffer.
 *
 * @param[out] p_buffer The buffer to fill.
 * @param[in] p_value The value to fill the buffer with. Note that only the 8
 *                    least significant bits are used. Therefore, if we try to
 *                    call this function with a p_value of 0xDEADBEEF, the
 *                    buffer will be filled with 0xEF bytes.
 * @param[in] p_size The number of bytes to set in the buffer.
 *
 * @returns A pointer to the destination buffer (p_destination).
 */
void *kmemset(void *p_buffer, int p_value, size_t p_size);

/**
 * @brief Puts the given character on the kernel standard output.
 *
 * @param[in] p_character The character to put on the kernel standard output.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval EOF if an error occurred.
 */
int kputchar(int p_character);

#endif
