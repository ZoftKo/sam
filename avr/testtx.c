#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "tx.h"

#define START_BTN_PIN PORTD3
#define START_BTN (PIND & (1 << START_BTN_PIN))

const uint8_t DEBOUNCE_WAIT = 32;

uint8_t rz = 0;
uint8_t transmitting = 0;

uint8_t message[] = {'H', 'e', 'l', 'l', 'o'};
struct Frame frame = {
    .address = {.to = 0x07, .from = 0x01}, .dinfo = {.type = ASCII, .size = 5}, .payload = message, .crc = 0x5555};

int main() {
    /**
     * 8-bit Timer0 setup
     * Generate interrupts at a 1kHz rate with a 64 bit pre-scaler.
     * Enable interrupts when the counter reaches the value of OCR0A.
     * */
    OCR0A = 64;
    TCCR0B = 0x03;
    TIMSK0 = 0x02;

    /**
     * IO setup
     * D2 is HIGH when a transmission is taking place. Mostly used for user feedback (e.g. light up an LED)
     * D3 (INT1) is an input with its pull-up enabled. When clicked, a transmission cycle begins
     * D4 is the transmitter's clock
     * D5 outputs the transmitter's bits
     * */
    PORTD = 0x08;
    DDRD = 0x34;

    PORTB = 0x00;
    DDRB = 0x20;

    tx_setup(&PORTD, PORTD5);

    _delay_ms(200);
    sei();
    while (1) {
        if (START_BTN == 0) {
            _delay_ms(DEBOUNCE_WAIT);
            if (START_BTN == 0) {
                while (START_BTN == 0)
                    ;
                tx_frame(frame);
                PORTD |= (1 << PORTD2);
            }
        }
        if (transmitting == 0) {
            PORTD &= ~(1 << PORTD2);
        }
    }
}

ISR(TIMER0_COMPA_vect) {
    if (rz == 0) {
        transmitting = tx_next();
        rz = 1;
    } else {
        PORTD &= ~(1 << PORTD5);
        rz = 0;
    }
    PIND |= 0x10;
}
