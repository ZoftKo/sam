#include "rx.h"

#include "bits.h"

static const volatile uint8_t *rxport;
static uint8_t rxpin;
static uint8_t buffer;
static struct Frame *rxframe;
static enum RxState state;
static void (*rx_on_error)(void);

enum RxSection { SECTION_ADDRESS, SECTION_INFO, SECTION_PAYLOAD, SECTION_CRC, SECTION_END };
static enum RxSection current_data;

static uint8_t zero_count, one_count;

void rx_reset(void) {
    state = IDLE;
    one_count = 0;
    zero_count = 0;
    current_data = SECTION_ADDRESS;
    buffer = 0;
}

void rx_setup(
    const volatile uint8_t *port, uint8_t pin, struct Frame *frame, void (*on_error)(void)
) {
    rxport = port;
    rxpin = pin;
    rxframe = frame;
    rx_on_error = on_error;
    rx_reset();
}

uint8_t rx_bit(void) {
    uint8_t value = read_bit(rxport, rxpin);
    if (one_count == ONE_LIMIT) {
        if (value == 0) {
            one_count = 0;
            return 1;
        } else {
            return 2;
        }
    }
    if (zero_count == ZERO_LIMIT) {
        if (value == 1) {
            zero_count = 0;
            return 1;
        } else {
            return 2;
        }
    }

    if (value == 1) {
        one_count++;
        zero_count = 0;
    } else {
        zero_count++;
        one_count = 0;
    }

    buffer = buffer << 1;
    buffer |= value;

    return 0;
}

uint8_t rx_byte(void) {
    static uint8_t received_bits = 0;

    switch (rx_bit()) {
        case 0:
            received_bits++;
            if (received_bits == 8) {
                received_bits = 0;
                return 0;
            }
            break;
        case 2:
            state = ERROR;
            rx_on_error();
        default:
            break;
    }

    return 1;
}

uint8_t seek_start_sequence(void) {
    static uint8_t received_ones = 0;
    if (read_bit(rxport, rxpin)) {
        received_ones++;
    } else {
        received_ones = 0;
    }

    if (received_ones == 8) {
        received_ones = 0;
        return 0;
    } else {
        return 1;
    }
}

uint8_t rx_payload(void) {
    static uint8_t counter = 0;
    switch (current_data) {
        case SECTION_ADDRESS:
            if (rx_byte() == 0) {
                rxframe->address.to = (buffer & 0xF0) >> 4;
                rxframe->address.from = (buffer & 0x0F);
                current_data = SECTION_INFO;
            }
            break;
        case SECTION_INFO:
            if (rx_byte() == 0) {
                if (counter == 0) {
                    rxframe->dinfo.type = (buffer & 0xF0) >> 4;
                    rxframe->dinfo.size = (buffer & 0x0F) << 8;
                    counter++;
                } else if (counter == 1) {
                    rxframe->dinfo.size |= buffer;
                    current_data = SECTION_PAYLOAD;
                    counter = 0;
                }
            }
            break;
        case SECTION_PAYLOAD:
            if (rx_byte() == 0) {
                rxframe->payload[counter] = buffer;
                counter++;
                if (counter == rxframe->dinfo.size) {
                    counter = 0;
                    current_data = SECTION_CRC;
                }
            }
            break;
        case SECTION_CRC:
            if (rx_byte() == 0) {
                if (counter == 0) {
                    rxframe->crc = buffer;
                    rxframe->crc = rxframe->crc << 8;
                    counter++;
                } else if (counter == 1) {
                    rxframe->crc |= buffer;
                    current_data = SECTION_END;
                    counter = 0;
                }
            }
            break;
        case SECTION_END:
            break;
    }
    return current_data != SECTION_END;
}

uint8_t rx_next(void) {
    switch (state) {
        case IDLE:
            if (seek_start_sequence() == 0) {
                state = RECEIVING;
            }
            break;
        case RECEIVING:
            switch (rx_payload()) {
                case 0:
                    state = END;
                    break;
                case 2:
                    state = ERROR;
                    break;
                default:
                    break;
            }
            break;
        case END:
            break;
        case ERROR:
        default:
            rx_on_error();
    }

    return state != END;
}

uint8_t rx_buffer(void) { return buffer; }

enum RxState rx_state(void) { return state; }
