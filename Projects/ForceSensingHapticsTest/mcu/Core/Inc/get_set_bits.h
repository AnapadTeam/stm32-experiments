/**
 ******************************************************************************
 * @file           : get_set_bits.h
 * @brief          : Contains helper functions for settings/getting bits in a
 *                   pointer to a register/number.
 * @author         : Jacob peterson
 ******************************************************************************
 */

#ifndef __GET_SET_BITS_H
#define __GET_SET_BITS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief  Sets 'number' inside 'to_set' at the bits defined by MSB and LSB
 * @param  to_set: a pointer to the bits to set
 * @param  number: the number to set inside 'to_set'
 * @param  msb: the MSB (0 - 31) (inclusive)
 * @param  lsb: the LSB (0 - 31) (inclusive)
 * @retval None
 * @author Jacob Peterson
 */
static inline void set_bits(uint32_t *to_set, uint32_t number, uint8_t msb, uint8_t lsb) {
    uint32_t temp = *((volatile uint32_t *)to_set);
    uint32_t mask = (~0 << (msb + 1)) | ~(~0 << lsb);
    temp |= ~mask & (number << lsb);
    temp &= mask | (number << lsb);
    *((volatile uint32_t *)to_set) = temp;
}

/**
 * @brief  Sets 'bit' inside 'to_set' at index
 * @param  to_set: a pointer to the bits to set
 * @param  bit: the bit to set inside 'to_set'
 * @param  index: the index (0 - 31)
 * @retval None
 * @author Jacob Peterson
 */
static inline void set_bit(uint32_t *to_set, uint8_t bit, uint8_t index) { set_bits(to_set, bit, index, index); }

/**
 * @brief  Gets the bits inside 'to_get' at the bits defined by MSB and LSB
 * @param  to_get: a pointer to the bits to get
 * @param  msb: the MSB (0 - 31) (inclusive)
 * @param  lsb: the LSB (0 - 31) (inclusive)
 * @retval the bits
 * @author Jacob Peterson
 */
static inline uint32_t get_bits(uint32_t *to_get, uint8_t msb, uint8_t lsb) {
    uint32_t mask = (~0 << (msb + 1)) | ~(~0 << lsb);
    return (*((volatile uint32_t *)to_get) & ~mask) >> lsb;
}

/**
 * @brief  Gets the bit inside 'to_get' at the bits at the index
 * @param  to_get: a pointer to the bits to get
 * @param  index: the index (0 - 31)
 * @retval the bit
 * @author Jacob Peterson
 */
static inline uint32_t get_bit(uint32_t *to_get, uint8_t index) { return get_bits(to_get, index, index); }

#ifdef __cplusplus
}
#endif

#endif /* __GET_SET_BITS_H */
