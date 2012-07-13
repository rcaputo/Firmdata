/*
 * atomq.h
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#ifndef ATOMQ_H_
#define ATOMQ_H_

#include <stdint.h>
#include <stdbool.h>

struct atomq;

typedef void (atomq_cb)(volatile struct atomq *);

struct atomq {
	uint8_t numSlots;
	uint8_t slotSize;
	bool dirty;
	uint8_t headOffset;
	uint8_t tailOffset;
	atomq_cb *cbDeqeueReady;
	void *cbData;

	unsigned char storage[];
};

void atomq_init(void);
volatile struct atomq * atomq_alloc(uint8_t numSlots, uint8_t slotSize);
bool atomq_enqueue(volatile struct atomq *queue, bool shouldBlock, void *src);
bool atomq_dequeue(volatile struct atomq *queue, bool shouldBlock, void *dest);
uint8_t atomq_slots_ready(volatile struct atomq *queue);

#endif /* ATOMQ_H_ */
