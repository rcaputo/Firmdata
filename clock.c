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

#if CLOCK_HZ == 10000
#define COUNTER_COMPARE 25
#elif CLOCK_HZ == 1000
#define COUNTER_COMPARE 250
#error Invalid clock frequency specified
#endif

bool sendTimerOverflow;

void clock_set_sendTimerOverflow(bool shouldSend) {
	sendTimerOverflow = shouldSend;
}

void clock_pause(void) {
	TCCR0B &= ~( 1 << CS02 | 0 << CS01 | 1 << CS00 );
}

void clock_reset(void) {
	TCNT0 = 0;
}

void clock_stop(void) {
	clock_pause();
	clock_reset();
}

void clock_run(void) {
	//64 prescaler
	TCCR0B |= 0 << CS02 | 1 << CS01 | 1 << CS00;
}

uint8_t clock_get(void) {
	return TCNT0;
}

void clock_init(void) {
	//clear on match
//	TCCR0A |= 1 << WGM01 | 0 << WGM00;

	sendTimerOverflow = false;

	OCR0A = COUNTER_COMPARE;

	TIMSK0 |= 1 << OCIE0A | 1 << TOIE0;
}

ISR(TIMER0_COMPA_vect) {
	led_on();

	OCR0A += COUNTER_COMPARE;

	timer_tick();
}

ISR(TIMER0_OVF_vect) {
	static char buf = 0;

	led_on();

	if (sendTimerOverflow) {
		if (! message_send(31, false, &buf, 1)) {
			fault_fatal(FAULT_CLOCK_OVERFLOW_WOULD_BLOCK);
		}
	}

}

