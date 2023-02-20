#ifndef SAM_RX_H
#define SAM_RX_H

#include <stdint.h>

#include "frame.h"

enum RxState { IDLE, RECEIVING, ERROR, END };

void rx_setup(
    const volatile uint8_t *port, uint8_t pin, struct Frame *frame, void (*on_error)(void)
);
void rx_reset(void);
uint8_t rx_bit(void);
uint8_t rx_byte(void);
uint8_t rx_next(void);
uint8_t rx_buffer(void);
enum RxState rx_state(void);

#endif
