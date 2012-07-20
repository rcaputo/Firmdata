/*
 * session.c
 *
 *  Created on: Jul 18, 2012
 *      Author: Tyler
 */


#include "session.h"
#include "command.h"
#include "atomq.h"

#define SESSION_EVENT_QUEUE_SIZE 10

bool sessionStarted;

void session_init(void) {
	sessionStarted = 0;
}

void session_update(void) {
	static enum session_event currentEvent;

	command_update();
}
