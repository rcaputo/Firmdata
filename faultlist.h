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
#define FAULT_MESSAGE_COMMAND_NAME_INVALID 17
#define FAULT_MESSAGE_COMMAND_ARG_DEQUEUE_WOULD_BLOCK 18
#define FAULT_MESSAGE_COMMAND_NO_MATCH 19
#define FAULT_MESSAGE_COMMAND_RESPONSE_TOO_LARGE 20
#define FAULT_MESSAGE_COMMAND_RESPONSE_WOULD_BLOCK 21
#define FAULT_MESSAGE_COMMAND_GOT_BYTE_WOULD_BLOCK 22
#define FAULT_SESSION_CLOCK_OVERFLOW_WOULD_BLOCK 23
#define FAULT_TIMER_NO_SLOTS_AVAILABLE 24
#define FAULT_SESSION_HANDLE_EVENT_EXITED_SWITCH 25
#define FAULT_SESSION_EVENT_DELIVER_WOULD_BLOCK 26
#define FAULT_SESSION_PROCESSOR_OVERFLOW_WOULD_BLOCK 27
#define FAULT_SESSION_ADC_SAMPLE_READY_WOULD_BLOCK 28
#define FAULT_ADC_INVALID_PIN 29
#define FAULT_MESSAGE_COMMAND_SUBSCRIBE_WOULD_BLOCK 30
#define FAULT_SESSION_SUBSCRIPTION_MALLOC_FAILED 31
#define FAULT_SESSION_ADC_SAMPLE_READY_MISMATCH 32
#define FAULT_COMMAND_SERVO_WOULD_BLOCK 33
#define FAULT_ADC_NOT_READY 34

#endif /* FAULTLIST_H_ */
