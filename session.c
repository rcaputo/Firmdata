/*
 * session.c
 *
 *  Created on: Jul 18, 2012
 *      Author: Tyler
 */

#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "session.h"
#include "command.h"
#include "atomq.h"
#include "fault.h"
#include "message.h"
#include "timer.h"
#include "clock.h"

bool sessionStarted;
volatile struct atomq *eventQueue;
uint8_t broadcastTimer;

static void session_start_broadcast(void);
static void session_stop_broadcast(void);

void session_event_deliver(enum session_event event) {
	if (! atomq_enqueue(eventQueue, false, &event)) {
		fault_fatal(FAULT_SESSION_EVENT_DELIVER_WOULD_BLOCK);
	}
}

void session_handle_event_startSession(void) {
	session_stop_broadcast();
	clock_reset();

	command_send_response(COMMAND_NAME_SESSION_START, NULL, 0);
	clock_set_sendTimerOverflow(true);
}

void session_handle_event_endSession(void) {
	clock_set_sendTimerOverflow(false);
	clock_reset();

	command_send_response(COMMAND_NAME_SESSION_END, NULL, 0);
	session_start_broadcast();
}

static void session_handle_event(enum session_event currentEvent) {
	switch(currentEvent) {
		case session_event_startSession: return session_handle_event_startSession();
		case session_event_endSession: return session_handle_event_endSession();
	};

	fault_fatal(FAULT_SESSION_HANDLE_EVENT_EXITED_SWITCH);
}

void session_send_broadcast(void) {
	static uint8_t message = MESSAGE_SYSTEM_BEACON;
	printf("Firmdata v%s ", FIRMDATA_VERSION);

	message_send(MESSAGE_SYSTEM_CHANNEL, true, &message, sizeof(uint8_t));
}

void session_broadcast_timercb(volatile struct timer *p) {
	session_send_broadcast();
}

static void session_start_broadcast(void) {
	broadcastTimer = timer_create(CLOCK_HZ, 0, true, session_broadcast_timercb, NULL);

	session_send_broadcast();

	timer_start(broadcastTimer);
}

static void session_stop_broadcast(void) {
	timer_delete(broadcastTimer);
}

void session_update(void) {
	static enum session_event currentEvent;

	while (atomq_dequeue(eventQueue, false, &currentEvent)) {
		session_handle_event(currentEvent);
	}
}

void session_init(void) {
	sessionStarted = 0;

	eventQueue = atomq_alloc(SESSION_EVENT_QUEUE_SIZE, sizeof(enum session_event));

	session_start_broadcast();
}
