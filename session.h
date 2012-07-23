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
	session_event_sessionStart, session_event_sessionEnd,
	session_event_clockOverflow, session_event_processorCounterOverflow,
	session_event_subscribe,

	session_event_heartBeat, session_event_adcSampleReady,
};

#define SESSION_EVENT_DATA_LEN_MAX 6

struct session_event {
	enum session_eventId id;
	unsigned char data[SESSION_EVENT_DATA_LEN_MAX];
};

struct session_subscription {
	uint8_t pin;
	uint8_t channel;
	uint16_t timerTop;
	uint16_t timerOffset;
};

void session_init(void);
void session_update(void);

void session_event_deliver_sessionStart(void);
void session_event_deliver_sessionEnd(void);
void session_event_deliver_clockOverflow(void);
void session_event_deliver_processorCounterOverflow(void);
void session_event_deliver_heartBeat(void);
void session_event_deliver_adcSampleReady(uint8_t pin, uint8_t timeStamp, uint8_t data);
void session_event_deliver_subscribe(uint8_t pin, uint8_t channel, uint16_t timerTop, uint16_t timerOffset);


#endif /* SESSION_H_ */
