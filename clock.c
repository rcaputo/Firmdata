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
#include "adc.h"

#if CLOCK_TIMER_HZ == 1024
#define COUNTER_COMPARE 16
#elif CLOCK_TIMER_HZ == 2048
#define COUNTER_COMPARE 8
#elif CLOCK_TIMER_HZ == 4096
#define COUNTER_COMPARE 4
#elif CLOCK_TIMER_HZ == 8192
#define COUNTER_COMPARE 2
#else
#error Invalid clock frequency specified
#endif

void clock_pause(void) {
	TCCR2B &= ~( 1 << CS22 | 1 << CS21 | 1 << CS20 );
}

void clock_reset(void) {
	TCNT2 = 0;
}

void clock_stop(void) {
	clock_pause();
	clock_reset();
}

void clock_run(void) {
	//1024 prescaler
	TCCR2B |= 1 << CS22 | 1 << CS21 | 1 << CS20;
}

static inline void clock_did_overflow(void) {
	session_event_deliver_clockOverflow();
}

uint8_t clock_get(void) {

	//check for an unsent overflow event and send it if needed
	//then clear the interrupt flag
	if (TIFR2 & 1 << TOV2) {
		TIFR2 &= ~(1 << TOV2);
		clock_did_overflow();
	}

	return TCNT2;
}

void clock_init(void) {
	OCR2A = COUNTER_COMPARE;

	TIMSK2 |= 1 << OCIE2A | 1 << TOIE2;
}

ISR(TIMER2_COMPA_vect) {
	processor_busy();

	OCR2A += COUNTER_COMPARE;

	timer_tick();

	sei();

	adc_check_sample();
	timer_run();
}

ISR(TIMER2_OVF_vect) {
	processor_busy();

	clock_did_overflow();
}

