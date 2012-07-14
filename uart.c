/*
 * uart.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/atomic.h>

#include "config.h"
#include "atomq.h"
#include "fault.h"
#include "message.h"
#include "command.h"
#include "led.h"

volatile struct atomq *inputBuf;
volatile struct atomq *outputBuf;

#define UART_UDRE_ENABLE (UCSR0B |= _BV(UDRIE0))
#define UART_UDRE_DISABLE (UCSR0B &= ~_BV(UDRIE0))
#define UART_RXC_ENABLE (UCSR0B |= _BV(RXCIE0))
#define UART_RXC_DISABLE (UCSR0B &= !_BV(RXCIE0))

static bool uart_buffer_nb(volatile struct atomq *queue, void *src, uint8_t len) {
	static uint8_t sent;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (atomq_slots_available(queue) < len) {
			return false;
		}

		for(sent = 0; sent < len; sent++) {
			if (! atomq_enqueue(queue, false, src)) {
				fault_fatal(FAULT_UART_BUFFER_NB_WOULD_BLOCK);
			}

			src++;
		}
	}

	return true;
}

bool uart_buffer_output(bool shouldBlock, void *src, uint8_t len) {
	static bool retval;

	while(1) {
		retval = uart_buffer_nb(outputBuf, src, len);

		if (retval || ! shouldBlock) {
			return retval;
		}
	}

	return true;
}


void uart_outputBuf_didEnqueue(volatile struct atomq *queue) {
	UART_UDRE_ENABLE;
}

void uart_inputBuf_didEnqueue(volatile struct atomq *queue) {

}

void uart_init(void) {

#define BAUD UART_BPS
//defines UBRRL_VALUE, UBRRH_VALUE and USE_2X
#include <util/setbaud.h>

	outputBuf = atomq_alloc(UART_OUTPUT_BUF_LEN, sizeof(char));
	inputBuf = atomq_alloc(UART_INPUT_BUF_LEN, sizeof(char));

	outputBuf->cbDidEnqueue = uart_outputBuf_didEnqueue;

	UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */

    message_set_buffer(outputBuf);
    command_set_buffer(inputBuf);

    UART_RXC_ENABLE;

#undef BAUD
}

ISR(USART_UDRE_vect) {
	static unsigned char byte;

	led_on();

	if (! atomq_dequeue(outputBuf, false, &byte)) {
		UART_UDRE_DISABLE;
		return;
	}

	if (! UCSR0A & _BV(UDRE0)) {
		fault_fatal(FAULT_UART_UDRE_INTERRUPT_WOULD_BLOCK_ON_SEND);
	}

	UDR0 = byte;
}

ISR(USART_RX_vect) {
	static unsigned char byte;

	led_on();

	if (! UCSR0A & _BV(RXC0)) {
		fault_fatal(FAULT_UART_RX_ISR_WOULD_BLOCK_ON_RECEIVE);
	}

	byte = UDR0;

	if (! atomq_enqueue(inputBuf, false, &byte)) {
		fault_fatal(FAULT_UART_RX_ISR_WOULD_BLOCK_ON_BUFFER);
	}
}
