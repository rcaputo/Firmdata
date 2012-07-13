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

void main_init(void) {
	ioport_init();
	led_init();
	fault_init();
	atomq_init();
	uart_init();
}

int main(void) {
	main_init();

	sei();

	printf("What's up, Doc? ");

	while(1) {
		volatile static int i = 0;
		i++;
	}
}
