/*
 * session.c
 *
 *  Created on: Jul 18, 2012
 *      Author: Tyler
 */

#include <stdlib.h>
#include <stdio.h>
#include <util/atomic.h>
#include <string.h>

#include "config.h"
#include "session.h"
#include "command.h"
#include "atomq.h"
#include "fault.h"
#include "message.h"
#include "timer.h"
#include "clock.h"
#include "processor.h"
#include "adc.h"

bool sessionActive;
volatile struct atomq *eventQueue;
bool gotHeartBeat;
volatile uint8_t adcCurrentPin;
volatile uint8_t adcCurrentChannel;

static void session_start_broadcast(void);
static void session_handle_event_sessionEnd(volatile struct session_event *p);
static void session_watchdog_timer_cb(volatile struct timer *p);
void session_event_deliver_adcSampleReady(uint8_t port, uint8_t timeStamp, uint8_t data);
void session_subscription_timer_cb(volatile struct timer *p);

static void session_event_deliver(volatile struct session_event *event) {
		if (! atomq_enqueue(eventQueue, false, (void *)event)) {
			fault_fatal(FAULT_SESSION_EVENT_DELIVER_WOULD_BLOCK);
		}
}

void session_event_deliver_sessionStart(void) {
	static volatile struct session_event event = { session_event_sessionStart, { } };
	session_event_deliver(&event);
}

static void session_handle_event_sessionStart(volatile struct session_event *p) {
	uint8_t timer;

	timer_delete_all();

	timer = timer_create(SESSION_WATCHDOG_TIMEOUT, 0, true, session_watchdog_timer_cb, NULL);
	timer_start(timer);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		clock_reset();
		processor_counter_reset();
		atomq_reset(eventQueue);

		sessionActive = 1;
	}

	command_send_response(COMMAND_NAME_SESSION_START, NULL, 0);
}

void session_event_deliver_sessionEnd(void) {
	static volatile struct session_event event = { session_event_sessionEnd, { } };
	session_event_deliver(&event);
}

static void session_handle_event_sessionEnd(volatile struct session_event *p) {
	clock_reset();

	timer_delete_all();

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		sessionActive = 0;
		atomq_reset(eventQueue);
	}

	command_send_response(COMMAND_NAME_SESSION_END, NULL, 0);
	session_start_broadcast();
}

void session_event_deliver_clockOverflow(void) {
	static volatile struct session_event event = { session_event_clockOverflow, { } };
	session_event_deliver(&event);

}

static void session_handle_event_clockOverflow(volatile struct session_event *p) {
	static char buf = 0;

	if (! sessionActive) {
		return;
	}

	if (! message_send(31, false, &buf, 1)) {
			fault_fatal(FAULT_SESSION_CLOCK_OVERFLOW_WOULD_BLOCK);
	}
}

void session_event_deliver_processorCounterOverflow(void) {
	static volatile struct session_event event = { session_event_processorCounterOverflow, { } };
	session_event_deliver(&event);

}

static void session_handle_event_processorOverflow(volatile struct session_event *p) {
	static char buf = 3;

	if (! sessionActive) {
		return;
	}

	if (! message_send(31, false, &buf, 1)) {
		fault_fatal(FAULT_SESSION_PROCESSOR_OVERFLOW_WOULD_BLOCK);
	}
}

void session_event_deliver_heartBeat(void) {
	static volatile struct session_event event = { session_event_heartBeat, { } };
	session_event_deliver(&event);
}

static void session_handle_event_heartBeat(volatile struct session_event *p) {
	gotHeartBeat = true;
}

void session_event_deliver_adcSampleReady(uint8_t pin, uint8_t timeStamp, uint8_t data) {
	static volatile struct session_event event = { session_event_adcSampleReady, { } };

	event.data[0] = pin;
	event.data[1] = timeStamp;
	event.data[2] = data;

	session_event_deliver(&event);
}

static void session_handle_event_adcSampleReady(volatile struct session_event *currentEvent) {
	if (adcCurrentPin != currentEvent->data[0]) {
		fault_fatal(FAULT_SESSION_ADC_SAMPLE_READY_MISMATCH);
	}

	if (! message_send(adcCurrentChannel, false, (void *)&currentEvent->data[1], 2)) {
		fault_fatal(FAULT_SESSION_ADC_SAMPLE_READY_WOULD_BLOCK);
	}
}

void session_event_deliver_subscribe(uint8_t pin, uint8_t channel, uint16_t timerTop) {
	static volatile struct session_event event = { session_event_subscribe, { } };

	event.data[0] = pin;
	event.data[1] = channel;
	event.data[2] = timerTop & 255;
	event.data[3] = timerTop >> 8;

	session_event_deliver(&event);
}

static void session_handle_event_subscribe(volatile struct session_event *currentEvent) {
	struct session_subscription *subscription = malloc(sizeof(struct session_subscription));
	uint8_t timer;

	if (subscription == NULL) {
		fault_fatal(FAULT_SESSION_SUBSCRIPTION_MALLOC_FAILED);
	}

	subscription->pin = currentEvent->data[0];
	subscription->channel = currentEvent->data[1];

	subscription->timerTop = currentEvent->data[2] | currentEvent->data[3] << 8;

	command_send_response(COMMAND_NAME_SUBSCRIBE, NULL, 0);

	timer = timer_create(subscription->timerTop, 0, true, session_subscription_timer_cb, subscription);
	timer_start(timer);
}

static void session_handle_event(struct session_event *currentEvent) {
	switch(currentEvent->id) {
		case session_event_sessionStart: return session_handle_event_sessionStart(currentEvent);
		case session_event_sessionEnd: return session_handle_event_sessionEnd(currentEvent);
		case session_event_clockOverflow: return session_handle_event_clockOverflow(currentEvent);
		case session_event_processorCounterOverflow: return session_handle_event_processorOverflow(currentEvent);
		case session_event_heartBeat: return session_handle_event_heartBeat(currentEvent);
		case session_event_adcSampleReady: return session_handle_event_adcSampleReady(currentEvent);
		case session_event_subscribe: return session_handle_event_subscribe(currentEvent);
	};

	fault_fatal(FAULT_SESSION_HANDLE_EVENT_EXITED_SWITCH);
}

void session_send_broadcast(void) {
	static uint8_t message = MESSAGE_SYSTEM_BEACON;
	printf("Firmdata v%s ", FIRMDATA_VERSION);

	message_send(MESSAGE_SYSTEM_CHANNEL, true, &message, sizeof(uint8_t));
}

void session_broadcast_timer_cb(volatile struct timer *p) {
	session_send_broadcast();
}

void session_watchdog_timer_cb(volatile struct timer *p) {
	if (! gotHeartBeat) {
		session_handle_event_sessionEnd(NULL);
	}

	gotHeartBeat = false;
}

void session_subscription_timer_cb(volatile struct timer *p) {
	struct session_subscription *subscription;

	subscription = p->handlerArg;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (! adc_take_sample(subscription->pin)) {
			return;
		}

		adcCurrentPin = subscription->pin;
		adcCurrentChannel = subscription->channel;
	}

}

static void session_start_broadcast(void) {
	uint8_t timer;

	session_send_broadcast();

	timer = timer_create(CLOCK_HZ, 0, true, session_broadcast_timer_cb, NULL);
	timer_start(timer);
}

void session_update(void) {
	static struct session_event currentEvent;

	while (atomq_dequeue(eventQueue, false, &currentEvent)) {
		session_handle_event(&currentEvent);
	}
}

void session_init(void) {
	sessionActive = 0;
	gotHeartBeat = false;

	eventQueue = atomq_alloc(SESSION_EVENT_QUEUE_SIZE, sizeof(struct session_event));

	session_start_broadcast();
}
