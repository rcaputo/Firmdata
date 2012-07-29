/*
 * timer.h
 *
 *  Created on: Jul 13, 2012
 *      Author: Tyler
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>
#include <stdbool.h>

struct timer;

typedef void (timer_cb)(volatile struct timer *);

struct timer {
	bool taken;
	bool active;
	bool repeat;
	bool dirty;
	uint16_t offset;
	uint16_t count;
	uint16_t top;
	timer_cb *handler;
	void *handlerArg;
};

void timer_init(void);
void timer_tick(void);
uint8_t timer_create(uint16_t top, uint16_t offset, bool repeat, timer_cb *handler, void *handlerArg);
void timer_start(uint8_t timerId);
void timer_delete(uint8_t timerId);
void timer_pause(uint8_t timerId);
void timer_delete_all(void);
void timer_run(void);

#endif /* TIMER_H_ */
