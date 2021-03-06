/*
 * command.c
 *
 *  Created on: Jul 13, 2012
 *      Author: Tyler
 */

#include <stdlib.h>
#include <util/atomic.h>
#include <string.h>
#include <stdio.h>

#include "command.h"
#include "fault.h"
#include "atomq.h"
#include "message.h"
#include "config.h"
#include "session.h"
#include "pwm.h"

#define COMMAND_RESPONSE_MAX_LEN 6

volatile struct atomq *inputBuf;

bool command_send_response(uint8_t command, void *ret, uint8_t len) {
	static char buf[COMMAND_RESPONSE_MAX_LEN + 2];

	if (len > COMMAND_RESPONSE_MAX_LEN) {
		fault_fatal(FAULT_MESSAGE_COMMAND_RESPONSE_TOO_LARGE);
	}

	buf[0] = MESSAGE_SYSTEM_COMMAND_RESPONSE;
	buf[1] = command;

	memcpy(&buf[2], ret, len);

	if (! message_send(MESSAGE_SYSTEM_CHANNEL, false, buf, len + 2)) {
		fault_fatal(FAULT_MESSAGE_COMMAND_RESPONSE_WOULD_BLOCK);
	}

	return true;
}

static bool command_handle_session_start(uint8_t command, volatile struct atomq *p) {
	session_event_deliver_sessionStart();

	return true;
}

static bool command_handle_session_end(uint8_t command, volatile struct atomq *p) {
	session_event_deliver_sessionEnd();
	return true;
}

static bool command_handle_nop(uint8_t command, volatile struct atomq *p) {
	return true;
}

static bool command_handle_echo(uint8_t command, volatile struct atomq *p) {
	static char buf;

	if (! atomq_dequeue(p, false, &buf)) {
		return false;
	}

	command_send_response(command, &buf, 1);

	return true;
}

static bool command_handle_identify(uint8_t command, volatile struct atomq *p) {
	static char buf[] = "am328p";

	command_send_response(command, buf, 6);

	return true;
}

static bool command_handle_test(uint8_t command, volatile struct atomq *p) {
	static uint16_t checksum;
	static uint8_t buf, i;

	checksum = 0;

	if (atomq_slots_consumed(p) < 6) {
		return false;
	}

	for(i = 0; i < 6; i++) {
		if(! atomq_dequeue(p, false, &buf)) {
			fault_fatal(FAULT_MESSAGE_COMMAND_ARG_DEQUEUE_WOULD_BLOCK);
		}

		checksum += buf;
	}

	command_send_response(command, &checksum, sizeof(checksum));

	return true;
}

static bool command_handle_heartBeat(uint8_t command, volatile struct atomq *p) {
	session_event_deliver_heartBeat();

	return true;
}

static bool command_handle_subscribe(uint8_t command, volatile struct atomq *p) {
	unsigned char buf[6];
	uint16_t timerTopBuf, timerOffsetBuff;
	int i;

	if (atomq_slots_consumed(p) < 6) {
		return false;
	}

	for(i = 0; i < 6; i++) {
		if(! atomq_dequeue(p, false, &buf[i])) {
			fault_fatal(FAULT_MESSAGE_COMMAND_SUBSCRIBE_WOULD_BLOCK);
		}
	}

	timerTopBuf = buf[2] | buf[3] << 8;
	timerOffsetBuff = buf[4] | buf[5] << 8;

	session_event_deliver_subscribe(buf[0], buf[1], timerTopBuf, timerOffsetBuff);

	return true;
}

static bool command_handle_servo(uint8_t command, volatile struct atomq *p) {
	static unsigned char buf[3];
	static uint16_t newValue;
	static uint8_t servoNum;
	static int i;

	if (atomq_slots_consumed(p) < 3) {
		return false;
	}

	for(i = 0; i < 3; i++) {
		if (! atomq_dequeue(p, false, &buf[i])) {
			fault_fatal(FAULT_COMMAND_SERVO_WOULD_BLOCK);
		}
	}

	newValue = buf[0] | buf[1] << 8;
	servoNum = buf[2];

	pwm_set(servoNum, newValue);

	return true;
}

static bool command_handle(uint8_t command, volatile struct atomq *p) {
	switch(command) {
		case COMMAND_NAME_NOP: return command_handle_nop(command, p);
		case COMMAND_NAME_ECHO: return command_handle_echo(command, p);
		case COMMAND_NAME_IDENTIFY: return command_handle_identify(command, p);
		case COMMAND_NAME_TEST: return command_handle_test(command, p);
		case COMMAND_NAME_SESSION_START: return command_handle_session_start(command, p);
		case COMMAND_NAME_SESSION_END: return command_handle_session_end(command, p);
		case COMMAND_NAME_HEARTBEAT: return command_handle_heartBeat(command, p);
		case COMMAND_NAME_SUBSCRIBE: return command_handle_subscribe(command, p);
		case COMMAND_NAME_SERVO: return command_handle_servo(command, p);
		default: fault_fatal(FAULT_MESSAGE_COMMAND_NO_MATCH); break;
	}

	return false;
}

void command_got_byte(volatile struct atomq *p) {
	static bool gotCommand = false;
	static uint8_t command;

	if (! gotCommand) {
		if (! atomq_dequeue(p, false, &command)) {
			fault_fatal(FAULT_MESSAGE_COMMAND_GOT_BYTE_WOULD_BLOCK);
		}

		gotCommand = true;
	}

	if (command_handle(command, p)) {
		gotCommand = false;
	}
}

void command_set_buffer(volatile struct atomq *p) {
	inputBuf = p;
}

void command_update(void) {
	if (atomq_slots_consumed(inputBuf) > 0) {
		command_got_byte(inputBuf);
	}
}

void command_init(void) {

}
