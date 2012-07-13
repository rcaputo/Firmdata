/*
 * fault.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#include <util/delay.h>
#include <avr/interrupt.h>

#include "led.h"
#include "fault.h"

#define FAULT_BLINK_COUNT 5

void fault_fatal(uint8_t faultNum) {
	uint8_t i;

	//never going to return from here, stop all other processing
	cli();

	//just in case the LED got broken some how
	led_init();

	while(1) {
		for(i = 0; i < FAULT_BLINK_COUNT; i++) {
			led_on();
			_delay_ms(50);
			led_off();
			_delay_ms(50);
		}

		_delay_ms(1000);

		for(i = 0; i < faultNum; i++) {
			led_on();
			_delay_ms(200);
			led_off();
			_delay_ms(300);
		}

		_delay_ms(1000);
	}
}

void fault_init(void) {

}

ISR(BADISR_vect) {
	fault_fatal(FAULT_UNHANDLED_ISR);
}
