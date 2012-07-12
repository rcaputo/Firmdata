/*
 * fault.h
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#ifndef FAULT_H_
#define FAULT_H_

#include <stdint.h>

#include "faultlist.h"

void fault_init(void);
void fault_fatal(uint8_t);

#endif /* FAULT_H_ */
