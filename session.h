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

enum session_event {
	session_event_startSession, session_event_endSession,
};

void session_init(void);
void session_update(void);
void session_event_deliver(enum session_event event);

#endif /* SESSION_H_ */
