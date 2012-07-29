/*
 * processor.c
 *
 *  Created on: Jul 20, 2012
 *      Author: Tyler
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "processor.h"
#include "led.h"
#include "session.h"
#include "fault.h"

//1024 prescaler
//#define PROCESSOR_MEASURE_START TCCR0B |= 1 << CS02 | 0 << CS01 | 1 << CS00
//#define PROCESSOR_MEASURE_STOP TCCR0B &= ~(1 << CS02 | 1 << CS01 | 1 << CS00)

void processor_idle(void) {
//	PROCESSOR_MEASURE_STOP;
	TCCR0B &= ~(1 << CS02 | 1 << CS01 | 1 << CS00);
	led_off();
}

void processor_busy(void) {
//	PROCESSOR_MEASURE_START;
	//1024 prescaler
	TCCR0B |= 1 << CS02 | 0 << CS01 | 1 << CS00;
	led_on();
}

void processor_counter_reset(void) {
	TCNT0 = 0;
}

void processor_init(void) {
	TIMSK0 |= 1 << TOIE0;
}

ISR(TIMER0_OVF_vect) {
	processor_busy();

	session_event_deliver_processorCounterOverflow();
}
