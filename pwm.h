/*
 * pwm.h
 *
 *  Created on: Jul 24, 2012
 *      Author: Tyler
 */

#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

void pwm_init(void);
void pwm_set(uint8_t pwmOut, uint16_t compareValue);

#endif /* PWM_H_ */
