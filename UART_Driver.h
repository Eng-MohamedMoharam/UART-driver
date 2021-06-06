/*
 * UART_Driver.h
 *
 * Created: 4/10/2021 8:44:12 AM
 *  Author: m
 */

#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_

#include "Kernal.h"

#define  TX			1
#define  RX			0
#define  UART_DDR	DDRD

void uartInit(unsigned long baudRate, char* recStrBuffer, unsigned short int recStrBufferSize, char* recNumBuffer, unsigned short int recNumBufferSize);

void uartSendingCharacter (char txData);

void uartSendingAck (void);

BOOL uartSendingString (char *data);

char uartReceivingCharacter(void);

volatile char* uartReceivingString(void);

unsigned int uartSendingNumber(unsigned int num);

unsigned int uartReceivingNumber(void);

#endif /* UART_DRIVER_H_ */