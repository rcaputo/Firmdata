/*
 * ioport.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#ifndef IOPORT_C_
#define IOPORT_C_

#include <avr/io.h>
#include <stdlib.h>

#include "ioport.h"
#include "fault.h"

volatile static uint8_t * ioport_get_ddr(volatile uint8_t *port) {
	if (port == &PORTB) {
		return &DDRB;
	} else {
		fault_fatal(FAULT_IOPORT_INVALID_PORT_SPECIFIED);
	}

	return NULL;
}

void ioport_set_direction_f(volatile uint8_t *port, uint8_t pin, enum ioport_direction direction) {
	volatile uint8_t *ddr = ioport_get_ddr(port);

	*ddr &= ~_BV(pin);
	*ddr |= direction << pin;
}

enum ioport_direction ioport_get_direction_f(volatile uint8_t *port, uint8_t pin) {
	volatile uint8_t *ddr = ioport_get_ddr(port);

	return *ddr &= _BV(pin);
}

void ioport_set_pin_f(volatile uint8_t *port, uint8_t pin, bool value) {
	*port &= ~_BV(pin);
	*port |= value << pin;
}

bool ioport_get_pin_f(volatile uint8_t *port, uint8_t pin) {
	return *port | _BV(pin);
}

void ioport_init(void) {

}

#endif /* IOPORT_C_ */
