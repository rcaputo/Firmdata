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

volatile uint8_t currentSampleChannel;
volatile uint8_t currentSampleTime;
volatile bool adcDirty;

void adc_init(void) {
	//left adjust result for lower precision
	ADMUX |= 1 << ADLAR;
	// /32 prescaler
	ADCSRA |= 1 << ADPS2 | 0 << ADPS1 | 1 << ADPS0;

	ADMUX |= 1 << REFS0;

	//enable adc
	ADCSRA |= 1 << ADEN;

	//the first sample takes longer so we'll do it now while the hardware is being initialized
	ADCSRA |= 1 << ADSC;

	while(ADCSRA & 1 << ADSC) {
		//busy wait for the first conversion to complete
	}

	adcDirty = false;
}

void adc_check_sample(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (adcDirty && ! (ADCSRA & 1 << ADSC)) {
			ADCSRA &= ~(1 << ADIF | 1 << ADIE);

			session_event_deliver_adcSampleReady(currentSampleChannel, currentSampleTime, ADCH);
			adcDirty = false;
		}

	}
}

bool adc_take_sample(uint8_t pin, uint8_t channel) {
	if (pin > 8) {
		fault_fatal(FAULT_ADC_INVALID_PIN);
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (ADCSRA & 1 << ADSC) {
			//adc is still performing conversion
			return false;
		}

		adc_check_sample();

		ADMUX &= ~15;
		ADMUX |= pin;

		currentSampleChannel = channel;
		currentSampleTime = clock_get();
		adcDirty = true;

		ADCSRA &= ~(1 << ADIF);
		ADCSRA |= 1 << ADSC | 1 << ADIE;
	}

	return true;
}

ISR(ADC_vect) {
	adc_check_sample();
	ADCSRA &= ~(1 << ADIE);
}
