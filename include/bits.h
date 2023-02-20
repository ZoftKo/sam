#ifndef SAM_BITS_H
#define SAM_BITS_H

#include <stdint.h>

/*
 * Read and return the selected bit's value
 * @param address Contains the bit to be read
 * @param bit Position of the bit to be read
 * */
uint8_t read_bit(const volatile uint8_t *address, uint8_t bit);

/**
 * Set the byte's N bit to 1
 * @param address Byte to be manipulated
 * @param bit Position of the bit to be turned on
 */
void bit_on(volatile uint8_t *address, uint8_t bit);

/**
 * Set the byte's N bit to 0
 * @param address Byte to be manipulated
 * @param bit  Position of the bit to be turned off
 */
void bit_off(volatile uint8_t *address, uint8_t bit);

/**
 * Set the byte's nibble with offset N to that of the value's high nibble (0xF0)
 * @param address Byte to be manipulated
 * @param value Value (only high nibble) that will be set
 * @param offset The 4 bits with offset (counting from LSB to MSB - right to left) to be set
 */
void set_high_nibble(volatile uint8_t *address, uint8_t value, uint8_t offset);

/**
 * Set the byte's nibble with offset N to that of the value's low nibble (0x0F)
 * @param address Byte to be manipulated
 * @param value Value (only low nibble) that will be set
 * @param offset The 4 bits with offset (counting from LSB to MSB - right to left) to be set
 */
void set_low_nibble(volatile uint8_t *address, uint8_t value, uint8_t offset);

#endif
