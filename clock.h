/*
 * clock.h
 *
 *  Created on: Jul 13, 2012
 *      Author: Tyler
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdint.h>
#include <stdbool.h>

void clock_pause(void);
void clock_stop(void);
void clock_run(void);
void clock_init(void);
uint8_t clock_get(void);
void clock_reset(void);

#endif /* CLOCK_H_ */
