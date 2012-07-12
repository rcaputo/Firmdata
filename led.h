/*
 * led.h
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#ifndef LED_H_
#define LED_H_

#include <stdbool.h>

void led_init(void);
void led_on(void);
void led_off(void);
bool led_get(void);
void led_set(bool);

#endif /* LED_H_ */
