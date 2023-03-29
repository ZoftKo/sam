// clang-format off
#include "config.h"
// clang-format on

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#include "bits.h"
#include "lcd.h"
#include "tx.h"
#include "usart.h"

#define SET_CHAR_BTN PINC1
#define ERASE_CHAR_BTN PINC5
#define TX_BTN PINC4

#define ENCODER_A PIND3
#define ENCODER_B PIND2
#define DISABLE_ENCODER EIMSK = 0x00
#define ENABLE_ENCODER EIMSK = 0x03

#define CLOCK_PIN PIND4
#define DATA_PIN PIND5

#define AVAILABLE_CHAR_COUNT 37

// clang-format off
const char AVAILABLE_CHARS[AVAILABLE_CHAR_COUNT] = {
    ' ', 'a', 'b', 'c', 'd','e', 'f', 'g', 'h', 'i',
'j', 'k', 'l', 'm', 'n', 'o','p', 'q', 'r', 's',
    't','u', 'v', 'w', 'x', 'y', 'z','0', '1',
    '2', '3', '4', '5', '6', '7', '8', '9'
};
// clang-format on

char text_buffer[33] = "tx hello";
char print_buffer[32] = {0};

uint8_t is_rz = 0;

struct EncoderStatus {
    uint8_t update_char : 1;
} encoder_status;

struct Frame frame = {
    .address = {.from = 0x01, .to = 0x07},
    .dinfo = {.type = ASCII, .size = 0},
    .payload = (unsigned char *)text_buffer,
    .crc = 0x55};
struct Display display = {
    .rs_port = &PORTB,
    .rs_bit = PINB4,
    .en_port = &PORTD,
    .en_bit = PIND7,
    .data_port = &PORTB,
    .data_shift = 0};

char current_char = 0;
uint8_t tx_running = 0;

void next_char(void) {
    if (current_char == (AVAILABLE_CHAR_COUNT - 1)) {
        current_char = 0;
    } else {
        current_char++;
    }
}

void prev_char(void) {
    if (current_char == 0) {
        current_char = AVAILABLE_CHAR_COUNT - 1;
    } else {
        current_char--;
    }
}

uint8_t was_clicked(volatile uint8_t *address, uint8_t pin) {
    if (read_bit(address, pin) == 0) {
        _delay_ms(DEBOUNCE_DELAY);
        if (read_bit(address, pin) == 0) {
            while (read_bit(address, pin) == 0)
                ;
            return 1;
        }
    }

    return 0;
}

void print_tx_data(void) {
    stwrite_usart("----\r\nData To Be Transmitted:\r\n");
    snprintf(
        print_buffer, 32, "To: 0x%02x\r\nFrom: 0x%02x\r\n", frame.address.to, frame.address.from
    );
    stwrite_usart(print_buffer);
    snprintf(print_buffer, 32, "Type: 0x%02x\r\nSize: %d\r\n", frame.dinfo.type, frame.dinfo.size);
    stwrite_usart(print_buffer);
    stwrite_usart("Payload:\r\n");
    stwrite_usart((char *)frame.payload);
    snprintf(print_buffer, 32, "\r\nCRC: 0x%02x\r\n", frame.crc);
    stwrite_usart(print_buffer);
}

void on_error(void) { TIMSK0 = 0x00; }

int main(void) {
    /**
     * 8-bit Timer0 setup
     * Generate interrupts at a 1kHz rate with a 64 bit pre-scaler.
     * Enable interrupts when the counter reaches the value of OCR0A.
     * */
    OCR0A = 64;
    TCCR0B = 0x03;
    TIMSK0 = 0x00;

    PORTB = 0x00;
    DDRB = 0xFF;

    PORTC = 0xFF;
    DDRC = 0x00;

    ENABLE_ENCODER;
    EICRA = 0x0A;
    PORTD = 0x0C;
    DDRD = 0xF3;

    tx_setup(&PORTD, DATA_PIN, &on_error);

    lcd_setup(&display);
    lcd_init(0x0D);
    lcd_clear_disp();
    lcd_wr_str((unsigned char *)text_buffer);

    start_usart(9600);

    sei();
    while (1) {
        if (encoder_status.update_char) {
            encoder_status.update_char = 0;
            lcd_wr_char(AVAILABLE_CHARS[(unsigned char)current_char]);
            lcd_step_cursor_left();
            ENABLE_ENCODER;
        }
        if (was_clicked(&PINC, ERASE_CHAR_BTN)) {
            text_buffer[lcd_cursor_pos()] = '\0';
            lcd_wr_char(' ');
            lcd_step_cursor_left();
            lcd_step_cursor_left();
            current_char = 0;
        }
        if (was_clicked(&PINC, SET_CHAR_BTN)) {
            text_buffer[lcd_cursor_pos()] = AVAILABLE_CHARS[(unsigned char)current_char];
            lcd_step_cursor_right();
            current_char = 0;
        }
        if (was_clicked(&PINC, TX_BTN)) {
            frame.dinfo.size = lcd_cursor_pos();
            print_tx_data();

            tx_frame(&frame);
            tx_running = 1;
            TIMSK0 = 0x02;
        }
    }
}

ISR(TIMER0_COMPA_vect) {
    if (tx_running == 0) {
        TIMSK0 = 0x00;
        bit_off(&PORTD, CLOCK_PIN);
        bit_off(&PORTD, DATA_PIN);
        is_rz = 0;
    } else if (is_rz == 1) {
        bit_off(&PORTD, DATA_PIN);
        bit_off(&PORTD, CLOCK_PIN);
        is_rz = 0;
    } else {
        tx_running = tx_next();
        bit_on(&PORTD, CLOCK_PIN);
        is_rz = 1;
    }
}

ISR(INT0_vect) {
    if (read_bit(&PIND, ENCODER_A) == 0) {
        DISABLE_ENCODER;
        next_char();
        encoder_status.update_char = 1;
    }
}

ISR(INT1_vect) {
    if (read_bit(&PIND, ENCODER_B) == 0) {
        DISABLE_ENCODER;
        prev_char();
        encoder_status.update_char = 1;
    }
}
