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

//256 prescaler
#define PROCESSOR_MEASURE_START TCCR2B |= 1 << CS22 | 1 << CS21 | 0 << CS20
#define PROCESSOR_MEASURE_STOP TCCR2B &= ~(1 << CS22 | 1 << CS21 | 1 << CS20)

void processor_idle(void) {
	PROCESSOR_MEASURE_STOP;
	led_off();
}

void processor_busy(void) {
	PROCESSOR_MEASURE_START;
	led_on();
}

void processor_counter_reset(void) {
	TCNT2 = 0;
}

void processor_init(void) {
	TIMSK2 |= 1 << TOIE2;
}

ISR(TIMER2_OVF_vect) {
	processor_busy();

	session_event_deliver(session_event_processorCounterOverflow);
}
