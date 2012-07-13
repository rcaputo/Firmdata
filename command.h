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

#define COMMAND_NAME_NOP 0
#define COMMAND_NAME_START 1

#endif /* COMMAND_H_ */
