/*
 * main.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#include <util/delay.h>

#include "fault.h"
#include "led.h"
#include "ioport.h"

void main_init(void) {
	ioport_init();
	led_init();
}

int main(void) {

	main_init();

	fault_fatal(FAULT_TEST);
}
