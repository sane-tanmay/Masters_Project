
/*
* XBeeS1_DSR.c
*
* Created: 5/20/2014 5:24:51 PM
*  Author:Tanmay Sane
*/
#include "Motesquito.h"
#include <avr/io.h>
#include <util/delay.h>
#include "Xbee.h"
#include "DSR.h"
#include <avr/interrupt.h>

char test[] = {'T', 'E', 'S', 'T'};
int cb;

void gas()
{	RxPacket rx_data = getPacket();
	// Echo packet back
	int i;
	char buff[rx_data.data_len];
	char temp;
	for(i=0;i<rx_data.data_len;i++)
	{	temp = (rx_data.data[i]);
		buff[i] = temp;
	}
	TX_Request_16bit(0x01, rx_data.source_addr_16bit, 0, buff, rx_data.data_len);
}

//DSR Test code
int main(void)
{
	motesquitoInit();//Set up Uart
	// setCoord();
	initDSR();
	char message[]={'M','O','T','E','S','Q','U','I','T','O'};
	int msg_status=Transmit_DSR(4,message,10);
	if (msg_status==1)
	{
		//Success
		PORTD=0x80;
		wait_sec(5);
		PORTD=0x00;
	}

	if (msg_status==2)
	{
		for (int i=0;i<20;i++)
		{  	PORTD=0x80;
			_delay_ms(1000);
			PORTD=0x00;
			_delay_ms(1000);
		}
	}

	if (msg_status==3)
	{
		for (int i=0;i<20;i++)
		{	PORTD=0x80;
			wait_sec(1);
			PORTD=0x00;
			wait_sec(1);
		}
		
	}
	while(1)
	{ }
}
