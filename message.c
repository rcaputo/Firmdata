/*
 * message.c
 *
 *  Created on: Jul 13, 2012
 *      Author: Tyler
 */

#include <util/atomic.h>
#include <stdlib.h>
#include <stdio.h>

#include "message.h"
#include "fault.h"

volatile struct atomq *messageBuf;

#define MESSAGE_HEADER_LEN 1
#define MESSAGE_MAX_SIZE 7

void message_set_buffer(volatile struct atomq *queue) {
	messageBuf = queue;
}

static bool message_send_nb(uint8_t port, void *src, uint8_t len) {
	static uint8_t i, header;

	if (len > MESSAGE_MAX_SIZE) {
		fault_fatal(FAULT_MESSAGE_MAX_LENGTH_EXCEEDED);
	}

	header = port << 5;
	header += len;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (messageBuf == NULL) {
			fault_fatal(FAULT_MESSAGE_BUFFER_WAS_NULL);
		}

		if (atomq_slots_ready(messageBuf) < len + MESSAGE_HEADER_LEN) {
			return false;
		}

		if (! atomq_enqueue(messageBuf, false, &header)) {
			fault_fatal(FAULT_MESSAGE_HEADER_SEND_WOULD_BLOCK_ON_ENQUEUE);
		}

		for(i = 0; i < len; i++) {
			if (! atomq_enqueue(messageBuf, false, src)) {
				fault_fatal(FAULT_MESSAGE_HEADER_BODY_WOULD_BLOCK_ON_ENQUEUE);
			}

			src++;
		}
	}

	return true;
}

bool message_send(uint8_t port, bool shouldBlock, void *src, uint8_t len) {
	static bool retval;

	while(1) {
		retval = message_send_nb(port, src, len);

		if (retval || ! shouldBlock) {
			return retval;
		}
	}

	return true;
}

static int message_putchar_stdio(char c, FILE *fh) {
	message_send(0, true, &c, 1);
	return 0;
}


void message_init(void) {
    fdevopen(message_putchar_stdio, NULL);
}


