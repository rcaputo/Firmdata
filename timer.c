/*
 * timer.c
 *
 *  Created on: Jul 13, 2012
 *      Author: Tyler
 */

#include <string.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "config.h"
#include "timer.h"
#include "fault.h"

volatile struct timer timers[TIMER_NUM_SLOTS];

uint8_t timer_create(uint16_t top, uint16_t offset, bool repeat, timer_cb *handler, void *handlerArg) {
	uint8_t i;

	for(i = 0; i < TIMER_NUM_SLOTS; i++) {
		if (! timers[i].taken) {
			break;
		}
	}

	if (timers[i].taken) {
		//oops no available timers
		fault_fatal(FAULT_TIMER_NO_SLOTS_AVAILABLE);
	}

	timers[i].dirty = false;
	timers[i].taken = true;
	timers[i].offset = offset;
	timers[i].top = top;
	timers[i].repeat = repeat;
	timers[i].handler = handler;
	timers[i].handlerArg = handlerArg;

	return i;
}

void timer_start(uint8_t timerId) {
	timers[timerId].active = true;
}

void timer_pause(uint8_t timerId) {
	timers[timerId].active = false;
}

void timer_stop(uint8_t timerId) {
	timer_pause(timerId);
	timers[timerId].count = 0;
}

void timer_delete(uint8_t timerId) {
	if (timers[timerId].handlerArg != NULL) {
		free(timers[timerId].handlerArg);
	}

	timer_stop(timerId);
	timers[timerId].taken = false;
	timers[timerId].handler = NULL;
	timers[timerId].handlerArg = NULL;
}

void timer_delete_all(void) {
	static int i;

	for(i = 0; i < TIMER_NUM_SLOTS; i++) {
		timer_delete(i);
	}
}

static void timer_update(volatile struct timer *p) {
	if (! p->active) {
		return;
	}

	if (p->offset > 0) {
		p->offset--;
	} else if (++p->count >= p->top) {
		p->dirty = true;
		p->count = 0;
	}
}

void timer_tick(void) {
	static uint8_t i;

	for(i = 0; i < TIMER_NUM_SLOTS; i++) {
		timer_update(&timers[i]);
	}
}

void timer_run(void) {
	static uint8_t i;
	static bool busy = false;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (busy) {
			return;
		}

		busy = true;
	}

	for(i = 0; i < TIMER_NUM_SLOTS; i++) {
		if (timers[i].dirty) {
			if (timers[i].handler != NULL) {
				timers[i].handler(&timers[i]);
			}

			timers[i].dirty = false;
		}
	}

	busy = false;
}

void timer_init(void) {
	memset((void *)timers, 0, sizeof(timers));
}
