/*
 * faultlist.h
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#ifndef FAULTLIST_H_
#define FAULTLIST_H_

#define _FAULT_NONE 1
#define FAULT_TEST 3
#define FAULT_MAINLOOP_DID_EXIT 4
#define FAULT_IOPORT_INVALID_PORT_SPECIFIED 5
#define FAULT_ATOMQ_ALLOC_FAILED 6
#define FAULT_UART_BUFFER_NB_WOULD_BLOCK 7
#define FAULT_UART_UDRE_INTERRUPT_WOULD_BLOCK_ON_SEND 8
#define FAULT_UNHANDLED_ISR 9
#define FAULT_UART_RX_ISR_WOULD_BLOCK_ON_RECEIVE 10
#define FAULT_UART_RX_ISR_WOULD_BLOCK_ON_BUFFER 11
#define FAULT_MESSAGE_DATA_RECEIVED_WOULD_BLOCK 12
#define FAULT_MESSAGE_BUFFER_WAS_NULL 13
#define FAULT_MESSAGE_HEADER_SEND_WOULD_BLOCK_ON_ENQUEUE 14
#define FAULT_MESSAGE_HEADER_BODY_WOULD_BLOCK_ON_ENQUEUE 15
#define FAULT_MESSAGE_MAX_LENGTH_EXCEEDED 16

#endif /* FAULTLIST_H_ */