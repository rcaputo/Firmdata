/*
 * pwm.c
 *
 *  Created on: Jul 24, 2012
 *      Author: Tyler
 */

#include <avr/io.h>
#include <util/atomic.h>
#include "ioport.h"

#include "pwm.h"

void pwm_set(uint8_t servoNum, uint16_t compareValue) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (servoNum == 1) {
			OCR1A = compareValue;
		} else if (servoNum == 2) {
			OCR1B = compareValue;
		}
	}
}

void pwm_init() {
	//Clear output compare on  Match when up counting.
	//Set output compare on  Match when downcounting.
	TCCR1A |= 1 << COM1A1 | 0 << COM1A0;
	TCCR1A |= 1 << COM1B1 | 0 << COM1B0;

	TCCR1B |= 1 << WGM13 | 0 << WGM12;
	TCCR1A |= 0 << WGM11 | 0 << WGM10;

	//set 50hz refresh rate with prescaler and WGM mode
	ICR1 = 40000;

	//init to center position
	OCR1A = 1500;
	OCR1B = 1500;

	ioport_set_direction(PORTB, 1, ioport_directionOut);
	ioport_set_direction(PORTB, 2, ioport_directionOut);

	// start timer with /8 prescaler
	TCCR1B |= 0 << CS12 | 1 << CS11 | 0 << CS10;
}

