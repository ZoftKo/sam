#include "bits.h"

#include <stdint.h>

/**
 * These masks represent the bits that will be manipulated based on the offset (from 0 to 4)
 */
static uint8_t masks[] = {0x0F, 0x1E, 0x3C, 0x78, 0xF0};

uint8_t read_bit(const volatile uint8_t *address, uint8_t bit) {
    if ((*address & (1 << bit)) == 0) {
        return 0;
    } else {
        return 1;
    }
}

void bit_on(volatile uint8_t *address, uint8_t bit) { *address |= (1 << bit); }

void bit_off(volatile uint8_t *address, uint8_t bit) { *address &= ~(1 << bit); }

void set_high_nibble(volatile uint8_t *address, uint8_t value, uint8_t offset) {
    *address = (*address & ~masks[offset]) | ((value >> (4 - offset)) & masks[offset]);
}

void set_low_nibble(volatile uint8_t *address, uint8_t value, uint8_t offset) {
    *address = (*address & ~masks[offset]) | ((value << offset) & masks[offset]);
}
