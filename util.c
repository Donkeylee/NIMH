#include <stdio.h> 

#include <avr/io.h>

#include <avr/interrupt.h>
#include <avr/signal.h>
#include <string.h>
#include "test.h"
#include <util/delay.h> 


#define ONE_SEC 10

volatile UINT16 length = 0;
volatile char buffer[UART_BUF_SIZE] = {0};
extern volatile UINT16 g_what_to_do;
volatile UINT16 value = 0;
volatile UINT16 g_elapsed_sec = 0;



SIGNAL(SIG_OUTPUT_COMPARE1A)// Timer1 OC1A interrupt function : 100ms Event
{
	volatile static UINT16 wSEC_counter = 0;

	if(++wSEC_counter == ONE_SEC)
	{
		wSEC_counter = 0;
		g_elapsed_sec++;
		//PRINTF("g_elapsed_sec\r\n");
	}
	//controller is approx. 280 ms faster /hr
	//if 10 hrs  it would be 2.8sec gap
}

SIGNAL(SIG_USART_RECV)
{
	while(!(UCSR0A & 0x80));

	take_over(UDR0);
}



void putch(char TX)
{

	while(!(UCSR0A & 0x20));

	UDR0 = TX;
}

void PRINTF(char *data, ...)
{

	va_list args;
	char astr[80];
	char *p_str;

	va_start(args, data);
	(void)vsnprintf(astr, sizeof(astr), data, args);
	va_end(args);

	p_str = astr;

	while(*p_str)
	{
	  putch(*p_str);
	  p_str++;
	}
}

void take_over(char recived_data)
{
	UINT16 i;
	buffer[length++]= recived_data;


	if(recived_data != 0x08)
	{
		putch(recived_data);
	}

	if(buffer[length - 1] == 0x0D)
	{
		if(strstr(buffer, "d") != 0)
		{
			PRINTF("DIScharge\r\n");
			g_what_to_do = DISCHARGE;
		}
		else if(strstr(buffer, "c") != 0)
		{
			PRINTF("Charge\r\n");
			g_what_to_do = CHARGE;
		}
		else if(strstr(buffer, "s") != 0)
		{
			PRINTF("stop charging\r\n");
			g_what_to_do = STOP;
		}
		else if(strstr(buffer, "r") != 0)
		{
			PRINTF("Cycling mode\r\n");
			g_what_to_do = REPEAT;
		}
		else if(strstr(buffer, "t") != 0)
		{
			PRINTF("Test mode\r\n");
			g_what_to_do = TEST_MODE;
		}		
		else if(length == 1)
		{

			//do notthing
			//PRINTF("g_what_to_do = %d\r\n", g_what_to_do);
		}
		else
		{
			PRINTF("\r\nERROR ");
		}

		PRINTF("\r\nUART> ");


		for(i = 0 ; i < length ; i ++)
		{		
			buffer[i] = 0;		
		}
		length = 0;		
	}
	else if(length >= UART_BUF_SIZE)
	{
		PRINTF("\r\nERROR ");
		PRINTF("\r\nUART> ");
		length = 0;
	}
}



