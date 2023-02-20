// clang-format off
#include "config.h"
// clang-format on

#include "lcd.h"

#include <stdint.h>
#include <util/delay.h>

#include "bits.h"

static const struct Display *lcd_conf;
static uint8_t cursor_position = 0;

void toggle_enable(void) {
    bit_on(lcd_conf->en_port, lcd_conf->en_bit);
    _delay_ms(2);
    bit_off(lcd_conf->en_port, lcd_conf->en_bit);
}

void lcd_setup(const struct Display *display) {
    lcd_conf = display;
    lcd_init(lcd_conf->config);
}

void lcd_send(uint8_t rs, uint8_t value) {
    if (rs == 0) {
        bit_off(lcd_conf->rs_port, lcd_conf->rs_bit);
    } else {
        bit_on(lcd_conf->rs_port, lcd_conf->rs_bit);
    }

    set_high_nibble(lcd_conf->data_port, value, lcd_conf->data_shift);
    toggle_enable();

    set_low_nibble(lcd_conf->data_port, value, lcd_conf->data_shift);
    toggle_enable();
}

void lcd_init(uint8_t config) {
    lcd_send(0, 0x02);
    lcd_send(0, 0x28);
    lcd_send(0, config);
    lcd_send(0, 0x14);
    lcd_send(0, 0x01);
}

void lcd_wr_str(const unsigned char *str) {
    while (*str != '\0') {
        lcd_send(1, *str);
        cursor_position++;
        str++;
    }
}

void lcd_wr_char(unsigned char value) {
    lcd_send(1, value);
    cursor_position++;
}

void lcd_mv_cursor(uint8_t address) {
    if (address & 0x40) {
        cursor_position = 16 + (address & 0x0F);
    } else {
        cursor_position = address;
    }
    lcd_send(0, (0x80 | address));
}

uint8_t lcd_step_cursor_right(void) {
    if (cursor_position == 31) {
        return cursor_position;
    }

    if (cursor_position == 15) {
        lcd_mv_cursor(0x40);
    } else {
        lcd_send(0, 0x14);
        cursor_position++;
    }

    return cursor_position;
}

uint8_t lcd_step_cursor_left(void) {
    if (cursor_position == 0) {
        return cursor_position;
    }

    if (cursor_position == 16) {
        lcd_mv_cursor(0x0F);
    } else {
        lcd_send(0, 0x10);
        cursor_position--;
    }

    return cursor_position;
}

uint8_t lcd_cursor_pos(void) { return cursor_position; }

void lcd_clear_disp(void) {
    lcd_send(0, 0x01);
    cursor_position = 0;
}
