/*
 * uart.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>

#include "config.h"
#include "atomq.h"

struct atomq *inputBuf;
struct atomq *outputBuf;

void uart_putchar(char c) {
    loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    UDR0 = c;
}

static int uart_putchar_stdio(char c, FILE *fh) {
	uart_putchar(c);
	return 0;
}

void uart_init(void) {
#define BAUD UART_BPS
//defines UBRRL_VALUE, UBRRH_VALUE and USE_2X
#include <util/setbaud.h>

	UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */

    fdevopen(uart_putchar_stdio, NULL);
}

