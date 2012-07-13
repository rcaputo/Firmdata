/*
 * command.c
 *
 *  Created on: Jul 13, 2012
 *      Author: Tyler
 */

#include <stdlib.h>

#include "command.h"
#include "fault.h"
#include "atomq.h"

volatile struct atomq *buffer;

void command_got_byte(volatile struct atomq *p) {
	atomq_dequeue(buffer, false, NULL);
}

void command_set_buffer(volatile struct atomq *p) {
	buffer = p;

	buffer->cbDidEnqueue = command_got_byte;
}

void command_init(void) {

}
