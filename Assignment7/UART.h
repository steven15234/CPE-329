/*
 * UART.h
 *
 *  Created on: May 4, 2017
 *      Author: Derek
 */

#ifndef UART_H_
#define UART_H_

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

void UART0_init(uint32_t buffer_size);

//UART Read Interrupt
void EUSCIA0_IRQHandler(void);

//get the current UART buffer
char* UART_get_buffer();

//check receive buffer flag
int UART_get_flag();
void UART_reset_flag();

void UART_send(char* string, int newline);

void UART_send_char(char val);

#endif /* UART_H_ */
