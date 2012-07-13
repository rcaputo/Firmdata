/*
 * main.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "fault.h"
#include "led.h"
#include "ioport.h"
#include "atomq.h"
#include "uart.h"
#include "message.h"

void main_init(void) {
	ioport_init();
	led_init();
	fault_init();
	atomq_init();
	uart_init();
	message_init();
}

void main_run(void) {
	sei();

	while(1) {
		volatile static int foo = 1;
		foo++;

		printf("Hello there ");
	}
}

int main(void) {
	main_init();
	main_run();
	fault_fatal(FAULT_MAINLOOP_DID_EXIT);
}