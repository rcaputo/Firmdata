/*
 * main.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#include <util/delay.h>
#include <stdio.h>

#include "fault.h"
#include "led.h"
#include "ioport.h"
#include "atomq.h"
#include "uart.h"

void main_init(void) {
	ioport_init();
	led_init();
	fault_init();
	atomq_init();
	uart_init();
}

int main(void) {
	uint16_t number = 10000;
	uint16_t out;
	volatile struct atomq *queue = atomq_alloc(10, sizeof(number));
	main_init();

	atomq_enqueue(queue, true, &number);
	atomq_dequeue(queue, true, &out);

	printf("Got %i\n", out);
}
