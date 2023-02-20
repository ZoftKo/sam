#include "tx.h"

#include "bits.h"

enum STATE { IDLE, INIT, TRANSMIT };

static enum STATE state;
static enum HeaderType current_data;

static uint8_t one_count, zero_count;

static uint8_t txpin;
static volatile uint8_t *txport;
static void (*tx_on_error)(void);

static const struct Frame *txframe;

static void tx_reset(void) {
    state = IDLE;
    current_data = TO;
    one_count = 0;
    zero_count = 0;
}

static void tx_start_sequence(void) {
    bit_on(txport, txpin);
    one_count++;
    if (one_count == 8) {
        one_count = 0;
        state = TRANSMIT;
    }
}

static uint8_t tx_payload(void) {
    static uint16_t index = 0;

    if (tx_byte(txframe->payload[index]) == 0) {
        index++;
        if (index < txframe->dinfo.size) {
            return 1;
        } else {
            index = 0;
            return 0;
        }
    }

    return 1;
}

void tx_setup(volatile uint8_t *port, uint8_t pin, void (*on_error)(void)) {
    txport = port;
    txpin = pin;
    tx_on_error = on_error;
    tx_reset();
}

uint8_t tx_bit(uint8_t bit) {
    if (one_count == ONE_LIMIT) {
        bit_off(txport, txpin);
        one_count = 0;
        return 1;
    }
    if (zero_count == ZERO_LIMIT) {
        bit_on(txport, txpin);
        zero_count = 0;
        return 1;
    }

    if (bit == 1) {
        bit_on(txport, txpin);
        one_count++;
        zero_count = 0;
    }
    if (bit == 0) {
        bit_off(txport, txpin);
        zero_count++;
        one_count = 0;
    }

    return 0;
}

uint8_t tx_nibble(uint8_t nibble) {
    static int8_t counter = 3;

    uint8_t bit = (nibble >> counter) & 0x01;
    if (tx_bit(bit) == 0) {
        counter--;
    }
    if (counter < 0) {
        counter = 3;
        return 0;
    } else {
        return 1;
    }
}

uint8_t tx_byte(uint8_t byte) {
    static int8_t counter = 7;

    if (tx_bit((byte >> counter) & 0x01) == 0) {
        counter--;
    }
    if (counter < 0) {
        counter = 7;
        return 0;
    } else {
        return 1;
    }
}

static void tx_transmit(void) {
    static int8_t counter = 0;

    switch (current_data) {
        case TO:
            if (tx_nibble(txframe->address.to) == 0) {
                current_data = FROM;
            }
            break;
        case FROM:
            if (tx_nibble(txframe->address.from) == 0) {
                current_data = TYPE;
            }
            break;
        case TYPE:
            if (tx_nibble(txframe->dinfo.type) == 0) {
                current_data = SIZE;
            }
            break;
        case SIZE:
            if (counter == 0) {
                if (tx_nibble((txframe->dinfo.size >> 8) & 0x0F) == 0) {
                    counter = 1;
                }
            } else if (tx_byte(txframe->dinfo.size & 0xFF) == 0) {
                counter = 0;
                current_data = PAYLOAD;
            }
            break;
        case PAYLOAD:
            if (tx_payload() == 0) {
                current_data = CRC;
            }
            break;
        case CRC:
            if (counter == 0) {
                if (tx_byte(txframe->crc >> 8) == 0) {
                    counter = 1;
                }
            } else if (tx_byte(txframe->crc & 0xFF) == 0) {
                counter = 0;
                current_data = TO;
                state = IDLE;
            }
            break;
        default:
            tx_on_error();
    }
}

uint8_t tx_next(void) {
    switch (state) {
        case INIT:
            tx_start_sequence();
            break;
        case TRANSMIT:
            tx_transmit();
            break;
        case IDLE:
            break;
        default:
            tx_on_error();
    }

    return state != IDLE;
}

uint8_t tx_frame(const struct Frame *frame) {
    if (state == IDLE) {
        tx_reset();
        txframe = frame;
        state = INIT;
        return 0;
    } else {
        return 1;
    }
}
