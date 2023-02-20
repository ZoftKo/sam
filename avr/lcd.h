#ifndef SAM_LCD_H
#define SAM_LCD_H

#include <stdint.h>

struct Display {
    volatile uint8_t *data_port;
    volatile uint8_t *rs_port;
    volatile uint8_t *en_port;
    uint8_t data_shift;
    uint8_t rs_bit;
    uint8_t en_bit;
    uint8_t config;
};

void lcd_setup(const struct Display *display);
void lcd_send(uint8_t rs, uint8_t value);
void lcd_init(uint8_t config);
void lcd_wr_str(const unsigned char *str);
void lcd_wr_char(unsigned char value);
void lcd_mv_cursor(uint8_t address);
uint8_t lcd_step_cursor_right(void);
uint8_t lcd_step_cursor_left(void);
uint8_t lcd_cursor_pos(void);
void lcd_clear_disp(void);

#endif
