/*
 * command.h
 *
 *  Created on: Jul 13, 2012
 *      Author: Tyler
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include "atomq.h"

void command_init(void);
void command_set_buffer(volatile struct atomq *);
void command_init(void);
void command_update();
bool command_send_response(uint8_t command, void *ret, uint8_t len);

#define COMMAND_NAME_NOP 1
#define COMMAND_NAME_ECHO 2
#define COMMAND_NAME_IDENTIFY 3
#define COMMAND_NAME_TEST 4
#define COMMAND_NAME_SESSION_START 10
#define COMMAND_NAME_SESSION_END 11
#define COMMAND_NAME_HEARTBEAT 12

#endif /* COMMAND_H_ */
