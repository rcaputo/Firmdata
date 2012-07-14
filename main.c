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
#include "clock.h"
#include "command.h"
#include "timer.h"

uint8_t timerId;

void main_init(void) {
	ioport_init();
	led_init();
	fault_init();
	atomq_init();
	uart_init();
	message_init();
	command_init();
	clock_init();
	timer_init();
}

void main_run(void) {
	sei();

	clock_run();

	while(1) {
		led_off();
	}
}

int main(void) {
	main_init();
	main_run();
	fault_fatal(FAULT_MAINLOOP_DID_EXIT);
}
