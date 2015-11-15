#include "DSR.h"
#define ATMEGA328P

#ifdef ATMEGA328P
#include <avr/interrupt.h>
#include <avr/io.h>

void timerEvent();
void initTimer(void);
void initTimer(void)
{
	TCNT0 = 0;	// clear count value
	TCCR0B = 0x03;// Select clk/64
	TIMSK0 |= (1 << TOIE0);
}

ISR(TIMER0_OVF_vect)
{
timerEvent();
}

#endif