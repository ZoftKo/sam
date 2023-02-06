#include "tx.h"

enum STATE { IDLE, INIT, TRANSMIT };
enum DATA_TYPE { TO, FROM, TYPE, SIZE, PAYLOAD, CRC };

static enum STATE state;
static enum DATA_TYPE current_data;

const uint8_t ONE_LIMIT = 5;
const uint8_t ZERO_LIMIT = 4;

static uint8_t psym, one_count, zero_count;

static uint8_t _txpbit;
static volatile uint8_t *_txport;

static struct Frame _frame;

static void reset() {
    state = IDLE;
    current_data = TO;
    one_count = 0;
    zero_count = 0;
    psym = 2;
}

static void writebit(uint8_t bit) {
    if (bit == 1) {
        *_txport |= (1 << _txpbit);
    } else if (bit == 0) {
        *_txport &= ~(1 << _txpbit);
    }
}

static void tx_start_sequence() {
    if (one_count < 7) {
        writebit(1);
        one_count++;
    } else {
        writebit(0);
        one_count = 0;
        state = TRANSMIT;
    }
}

static uint8_t tx_payload() {
    static uint16_t index = 0;

    if (tx_byte(_frame.payload[index]) == 0) {
        index++;
        if (index < _frame.dinfo.size) {
            return 1;
        } else {
            return 0;
        }
    }

    return 1;
}

void tx_setup(volatile uint8_t *port, uint8_t txpbit) {
    _txport = port;
    _txpbit = txpbit;
    reset();
}

uint8_t tx_bit(uint8_t bit) {
    if (one_count == ONE_LIMIT) {
        writebit(0);
        one_count = 0;
        return 1;
    }
    if (zero_count == ZERO_LIMIT) {
        writebit(1);
        zero_count = 0;
        return 1;
    }

    if (bit == 1) {
        writebit(1);
        if (psym != 0) {
            one_count++;
        } else {
            one_count = 1;
        }
    }
    if (bit == 0) {
        writebit(0);
        if (psym != 1) {
            zero_count++;
        } else {
            zero_count = 1;
        }
    }
    psym = bit;

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

static void tx_transmit() {
    static int8_t counter = 0;

    switch (current_data) {
        case TO:
            if (tx_nibble(_frame.address.to) == 0) {
                current_data = FROM;
            }
            break;
        case FROM:
            if (tx_nibble(_frame.address.from) == 0) {
                current_data = TYPE;
            }
            break;
        case TYPE:
            if (tx_nibble(_frame.dinfo.type) == 0) {
                current_data = SIZE;
            }
            break;
        case SIZE:
            if (counter == 0) {
                if (tx_nibble((_frame.dinfo.size >> 8) & 0x0F) == 0) {
                    counter = 1;
                }
            } else if (tx_byte(_frame.dinfo.size & 0xFF) == 0) {
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
                if (tx_byte((_frame.crc >> 8) & 0xFF) == 0) {
                    counter = 1;
                }
            } else if (tx_byte(_frame.crc & 0xFF) == 0) {
                counter = 0;
                current_data = TO;
                state = IDLE;
            }
            break;
    }
}

uint8_t tx_next() {
    switch (state) {
        case INIT:
            tx_start_sequence();
            break;
        case TRANSMIT:
            tx_transmit();
            break;
        case IDLE:
            break;
    }

    return state != IDLE;
}

uint8_t tx_frame(struct Frame frame) {
    if (state == IDLE) {
        _frame = frame;
        state = INIT;
        return 0;
    } else {
        return 1;
    }
}
