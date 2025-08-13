/**
 * @file system_entropy.h
 * @brief System entropy sampling module header
 */

#ifndef SYSTEM_ENTROPY_H
#define SYSTEM_ENTROPY_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the system entropy subsystem
 */
void system_entropy_init(void);

/**
 * @brief Extract entropy data with maximum performance
 * 
 * @param buffer Buffer to store entropy data
 * @param size Size of buffer in bytes
 * @return Number of bytes extracted
 */
size_t system_entropy_extract_fast(uint8_t* buffer, size_t size);

/**
 * @brief Cleanup system entropy resources
 */
void system_entropy_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_ENTROPY_H