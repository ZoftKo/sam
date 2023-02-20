// clang-format off
#include "config.h"
// clang-format on

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bits.h"
#include "lcd.h"
#include "rx.h"
#include "usart.h"

#define CLOCK_PIN PIND2
#define DATA_PIN PIND3

struct Display display = {
    .rs_port = &PORTB,
    .rs_bit = PINB4,
    .en_port = &PORTD,
    .en_bit = PIND7,
    .data_port = &PORTB,
    .data_shift = 0};

char print_buffer[32];

unsigned char text_buffer[33] = {0};
struct Frame frame = {.address = {0}, .dinfo = {0}, .payload = text_buffer, .crc = 0};

void print_rx_data(void) {
    stwrite_usart("----\r\nData Received:\r\n");
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

void on_error(void) {
    EIMSK = 0x00;
    stwrite_usart("rx error\r\n");
    print_rx_data();
}

int main(void) {
    EICRA = 0x03;  // Interrupt on rising edge for INT0 (PD2)
    EIMSK = 0x01;

    PORTB = 0x00;
    DDRB = 0xFF;

    PORTD = 0x00;
    DDRD = 0xF3;

    rx_setup(&PIND, DATA_PIN, &frame, &on_error);

    lcd_setup(&display);
    lcd_init(0x0C);
    lcd_clear_disp();
    lcd_wr_str((unsigned char *)"rx idle");

    start_usart(9600);
    stwrite_usart("rx idle\r\n");

    sei();
    while (1) {
        if (rx_state() == END) {
            EIMSK = 0x00;
            bit_off(&PORTB, PINB5);
            print_rx_data();
            lcd_clear_disp();
            lcd_wr_str(frame.payload);
            memset(frame.payload, 0x00, 33);
            rx_reset();
            EIMSK = 0x01;
        }
    }
}

ISR(INT0_vect) {
    bit_on(&PINB, PINB5);
    rx_next();
}
