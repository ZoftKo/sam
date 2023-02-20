#ifndef SAM_USART_H
#define SAM_USART_H

#include <avr/io.h>

#include "config.h"

void start_usart(uint32_t baud);
void write_usart(char data);
void stwrite_usart(char *data);

#endif
