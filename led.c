/*
 * led.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#include "led.h"
#include "config.h"
#include "ioport.h"

void led_init(void) {
	ioport_set_direction(LED_PORT, LED_PIN, ioport_directionOut);
}

bool led_get(void) {
	return ioport_get_pin_f(&LED_PORT, LED_PIN);
}

void led_set(bool active) {
	ioport_set_pin(LED_PORT, LED_PIN, active);
}

void led_on(void) {
	led_set(true);
}

void led_off(void) {
	led_set(false);
}
