/*
 * adc.c
 *
 *  Created on: Jul 21, 2012
 *      Author: Tyler
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/atomic.h>

#include "adc.h"
#include "fault.h"
#include "session.h"
#include "clock.h"

volatile uint8_t currentSamplePin;
volatile uint8_t currentSampleTime;

void adc_init(void) {
	//left adjust result for lower precision
	ADMUX |= 1 << ADLAR;
	///128 prescaler
	ADCSRA |= 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0;

	ADMUX |= 1 << REFS0;

	//enable adc
	ADCSRA |= 1 << ADEN;

	//the first sample takes longer so we'll do it now while the hardware is being initialized
	ADCSRA |= 1 << ADSC;

	while(ADCSRA & 1 << ADSC) {
		//busy wait for the first conversion to complete
	}

}

bool adc_take_sample(uint8_t pin) {
	if (pin > 8) {
		fault_fatal(FAULT_ADC_INVALID_PIN);
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (ADCSRA & 1 << ADSC) {
			return false;
		}

		ADMUX &= ~15;
		ADMUX |= pin;

		currentSamplePin = pin;
		currentSampleTime = clock_get();

		ADCSRA &= ~(1 << ADIF);
		ADCSRA |= 1 << ADSC | 1 << ADIE;
	}

	return true;
}

ISR(ADC_vect) {
	ADCSRA &= ~(1 << ADIE);

	session_event_deliver_adcSampleReady(currentSamplePin, currentSampleTime, ADCH);
}
