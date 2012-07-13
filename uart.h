/*
 * uart.h
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#ifndef UART_H_
#define UART_H_

void uart_init(void);
bool uart_buffer_output(bool shouldBlock, void *src, uint8_t len);

#endif /* UART_H_ */
