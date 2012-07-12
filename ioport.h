/*
 * ioport.h
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#ifndef IOPORT_H_
#define IOPORT_H_

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

enum ioport_direction { ioport_directionIn = 0, ioport_directionOut = 1 };

void ioport_init();

#define ioport_set_direction(port, pin, direction) ioport_set_direction_f(&port, pin, direction)
void ioport_set_direction_f(volatile uint8_t *port, uint8_t pin, enum ioport_direction direction);

#define ioport_get_direction(port, pin) ioport_get_direction_f(&port, pin)
enum ioport_direction ioport_get_direction_f(volatile uint8_t *port, uint8_t pin);

#define ioport_set_pin(port, pin, value) ioport_set_pin_f(&port, pin, value)
void ioport_set_pin_f(volatile uint8_t *port, uint8_t pin, bool value);

#define ioport_get_pin(port, pin) ioport_get_pin_f(&port, pin);
bool ioport_get_pin_f(volatile uint8_t *port, uint8_t pin);

#endif /* IOPORT_H_ */
