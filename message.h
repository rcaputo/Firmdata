/*
 * message.h
 *
 *  Created on: Jul 13, 2012
 *      Author: Tyler
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdbool.h>

#include "atomq.h"

#define MESSAGE_SYSTEM_CHANNEL 31
#define MESSAGE_SYSTEM_CLOCK_OVERFLOW 0
#define MESSAGE_SYSTEM_COMMAND_RESPONSE 1
#define MESSAGE_SYSTEM_BEACON 2

void message_init(void);
void message_data_received(volatile struct atomq *);
bool message_send(uint8_t port, bool shouldBlock, void *src, uint8_t len);
void message_set_buffer(volatile struct atomq *queue);

#endif /* MESSAGE_H_ */
