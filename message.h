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

void message_init(void);
void message_data_received(volatile struct atomq *);
bool message_send(uint8_t port, bool shouldBlock, void *src, uint8_t len);
void message_set_buffer(volatile struct atomq *queue);

#endif /* MESSAGE_H_ */
