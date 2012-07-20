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

#define COMMAND_NAME_NOP 1
#define COMMAND_NAME_ECHO 2
#define COMMAND_NAME_IDENTIFY 3
#define COMMAND_NAME_TEST 4

#endif /* COMMAND_H_ */
