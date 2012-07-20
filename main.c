/*
 * main.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "config.h"
#include "fault.h"
#include "led.h"
#include "ioport.h"
#include "atomq.h"
#include "uart.h"
#include "message.h"
#include "clock.h"
#include "command.h"
#include "timer.h"
#include "session.h"

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
	session_init();
}

void main_update_subsystems(void) {
	command_update();
	session_update();
}

void main_sleep(void) {
	sleep_enable();
	led_off();
	sleep_cpu();
	sleep_disable();
}

void main_run(void) {
	led_on();

	sei();
	clock_run();

	while(1) {
		main_update_subsystems();
		main_sleep();
	}
}

int main(void) {
	main_init();
	main_run();
	fault_fatal(FAULT_MAINLOOP_DID_EXIT);
}
