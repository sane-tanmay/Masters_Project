/*
 * Motesquito.h
 *
 * Created: 3/27/2014 12:26:01 AM
 *  Author: Waron
 */ 


#ifndef MOTESQUITO_H_
#define MOTESQUITO_H_

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Xbee.h"

#define F_CPU 16000000	//be sure to adjust for various clock settings
#define FTDI_USART_BAUDRATE 9600
#define FTDI_BAUD_PRESCALE (((F_CPU / (FTDI_USART_BAUDRATE * 16UL))) - 1)

#define FTDI_UDR UDR0
#define FTDI_UART_BUFFER_IS_FULL() ((UCSR0A & (1 << UDRE0)) == 0)

void USART0_init(void);
void motesquitoInit(void);

#endif /* MOTESQUITO_H_ */