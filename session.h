/*
 * session.h
 *
 *  Created on: Jul 18, 2012
 *      Author: Tyler
 */

#ifndef SESSION_H_
#define SESSION_H_

#include <stdint.h>
#include <stdbool.h>

enum session_event {
	session_event_clockOverflow
};

void session_init(void);
void session_update(void);



#endif /* SESSION_H_ */
