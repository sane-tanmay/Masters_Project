/*
 * Motesquito.c
 *
 * Created: 3/27/2014 12:25:43 AM
 *  Author: Waron
 */ 
#include "Motesquito.h"
#include "Xbee.h"

	
void USART0_init()
{
	UBRR0L = FTDI_BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
	UBRR0H = (FTDI_BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register

	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01); // Use 8-bit character sizes
	UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);   // Turn on the transmission and reception circuitry
}
	
/* The following routines set the default streaming target for
 * the printf(_) function for USART0
 */
static int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
      uart_putchar('\r', stream);
     while(FTDI_UART_BUFFER_IS_FULL());
     FTDI_UDR = c;
     return 0;
}
		
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void motesquitoInit()
{
	stdout = &mystdout;  //set stdout to stream from USART0
	USART0_init();
	XbeeInit();
	sei();
}