/*
 * session.h
 *
 *  Created on: Jul 18, 2012
 *      Author: Tyler
 */

#ifndef SESSION_H_
#define SESSION_H_

#include <stdint.h>
#include <stdbool.h>

enum session_eventId {
	session_event_startSession, session_event_endSession,
	session_event_clockOverflow, session_event_processorCounterOverflow,

	session_event_test, session_event_heartBeat,
};

void session_init(void);
void session_update(void);
void session_event_deliver(enum session_eventId event);

#endif /* SESSION_H_ */
