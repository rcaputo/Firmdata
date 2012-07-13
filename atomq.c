/*
 * atomq.c
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#include <util/atomic.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "atomq.h"
#include "fault.h"

volatile struct atomq * atomq_alloc(uint8_t numSlots, uint8_t slotSize) {
	uint16_t numBytes = sizeof(struct atomq) + numSlots * slotSize;
	struct atomq *tmp = malloc(numBytes);

	if (tmp == NULL) {
		fault_fatal(FAULT_ATOMQ_ALLOC_FAILED);
	}

	memset((void *)tmp, 0, numBytes);

	tmp->numSlots = numSlots;
	tmp->slotSize = slotSize;

	return tmp;
}

bool atomq_enqueue_nb(volatile struct atomq *queue, void *src) {

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (queue->headOffset == queue->tailOffset && queue->dirty) {
			//no room left in queue
			return false;
		}

		memcpy((void *)&(queue->storage[queue->headOffset * queue->slotSize]), src, queue->slotSize);

		queue->headOffset++;
		queue->headOffset %= queue->numSlots;

		queue->dirty = true;
	}

	queue->cbDidEnqueue(queue);

	return true;
}

bool atomq_enqueue(volatile struct atomq *queue, bool shouldBlock, void *src) {
	static bool retval;

	while(1) {
		retval = atomq_enqueue_nb(queue, src);

		if (retval || ! shouldBlock) {
			return retval;
		}
	}

	return false;
}

bool atomq_dequeue_nb(volatile struct atomq *queue, void *dest) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (queue->tailOffset == queue->headOffset && ! queue->dirty) {
			//nothing exists in the queue
			return false;
		}

		memcpy(dest, (void *)&(queue->storage[queue->tailOffset * queue->slotSize]), queue->slotSize);

		queue->tailOffset++;
		queue->tailOffset %= queue->numSlots;

		if (queue->tailOffset == queue->headOffset) {
			queue->dirty = false;
		}
	}

	return true;
}

bool atomq_dequeue(volatile struct atomq *queue, bool shouldBlock, void *dest) {
	static bool retval;

	while(1) {
		retval = atomq_dequeue_nb(queue, dest);

		if (retval || ! shouldBlock) {
			return retval;
		}
	}

	return false;
}

static bool atomq_peek_nb(volatile struct atomq *queue, void *dest) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (queue->tailOffset == queue->headOffset && ! queue->dirty) {
			//nothing exists in the queue
			return false;
		}

		memcpy(dest, (void *)&(queue->storage[queue->tailOffset * queue->slotSize]), queue->slotSize);
	}

	return true;
}

bool atomq_peek(volatile struct atomq *queue, bool shouldBlock, void *dest) {
	static bool retval;

	while(1) {
		retval = atomq_peek_nb(queue, dest);

		if (retval || ! shouldBlock) {
			return retval;
		}
	}

	return false;
}

uint8_t atomq_slots_ready(volatile struct atomq *queue) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (queue->tailOffset == queue->headOffset) {
			if (queue->dirty) {
				return 0;
			} else {
				return queue->numSlots;
			}
		} else if (queue->tailOffset > queue->headOffset) {
			return queue->tailOffset - queue->headOffset;
		}
	}

	return queue->numSlots - (queue->headOffset - queue->tailOffset);
}

uint8_t atomq_slots_used(volatile struct atomq *queue) {
	return queue->numSlots - atomq_slots_ready(queue);
}

void atomq_init(void) {

}
