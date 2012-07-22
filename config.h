/*
 * config.h
 *
 *  Created on: Jul 12, 2012
 *      Author: Tyler
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define FIRMDATA_VERSION "0.0.1"

#define CLOCK_HZ 2500

#define LED_PORT PORTB
#define LED_PIN 5

#define UART_BPS 57600
#define UART_INPUT_BUF_LEN 64
#define UART_OUTPUT_BUF_LEN 255

#define TIMER_NUM_SLOTS 20

#define SESSION_EVENT_QUEUE_SIZE 10
#define SESSION_WATCHDOG_TIMEOUT CLOCK_HZ * 2

#endif /* CONFIG_H_ */
