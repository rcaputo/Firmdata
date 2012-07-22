/*
 * clock.c
 *
 *  Created on: Jul 13, 2012
 *      Author: Tyler
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "led.h"
#include "fault.h"
#include "message.h"
#include "timer.h"
#include "session.h"
#include "processor.h"

#if CLOCK_HZ == 2500
#define COUNTER_COMPARE 25
#else
#error Invalid clock frequency specified
#endif

void clock_pause(void) {
	TCCR0B &= ~( 1 << CS02 | 1 << CS01 | 1 << CS00 );
}

void clock_reset(void) {
	TCNT0 = 0;
}

void clock_stop(void) {
	clock_pause();
	clock_reset();
}

void clock_run(void) {
	//256 prescaler
	TCCR0B |= 1 << CS02 | 0 << CS01 | 0 << CS00;
}

uint8_t clock_get(void) {
	return TCNT0;
}

void clock_init(void) {
	OCR0A = COUNTER_COMPARE;

	TIMSK0 |= 1 << OCIE0A | 1 << TOIE0;
}

ISR(TIMER0_COMPA_vect) {
	processor_busy();

	OCR0A += COUNTER_COMPARE;

	timer_tick();
}

ISR(TIMER0_OVF_vect) {
	processor_busy();

	session_event_deliver_clockOverflow();
}

