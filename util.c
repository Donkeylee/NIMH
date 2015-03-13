#include <stdio.h> 

#include <avr/io.h>

#include <avr/interrupt.h>
#include <avr/signal.h>
#include <string.h>
#include <util/delay.h> 
#include "test.h"

#define ONE_SEC 10

volatile int length = 0;
volatile char buffer[256] = {0};
extern volatile int g_what_to_do;
volatile int value = 0;
volatile unsigned int g_elapsed_sec = 0;

SIGNAL(SIG_USART_RECV)
{
	while(!(UCSR0A & 0x80));

	take_over(UDR0);
}

SIGNAL(SIG_OUTPUT_COMPARE1A)// Timer1 OC1A interrupt function : 100ms Event
{
	volatile static int wSEC_counter = 0;

	if(++wSEC_counter == ONE_SEC)
	{
		wSEC_counter = 0;
		g_elapsed_sec++;
	}
	//controller is approx. 280 ms faster /hr
	//if 10 hrs  it would be 2.8sec gap
	

	
#if 0
	//*-----------Picture take frequency timer-----------*//
	if(++wSEC_counter > ONE_MINUTE) //if sectimer is over then minute.
	{
		wSEC_counter = 0; 
		wMIN_counter++; //add one minute.
	}
	//*--------------------------------------------------*//


	//*-----------Push & pull button delay timer-----------*//
	if(wPWR_counter < SEC_20) //if button timer is less then 20 sec.
		wPWR_counter++;
	//*----------------------------------------------------*//
#endif
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
	buffer[length++]= recived_data;

	char *pStr;
	int i;


	if(recived_data != 0x08)
	{
		putch(recived_data);
	}

	if(buffer[length - 1] == 0x0D)
	{

		if(buffer[length - 2] == '?')
		{
			PRINTF("\r\nHELP ");
			PRINTF("\r\n1. ");
			PRINTF("\r\n2. ");
			PRINTF("\r\n3. ");
			PRINTF("\r\n4. ");
			PRINTF("\r\n5. ");
			PRINTF("\r\n6. ");			
		}
		else if(strstr(buffer, "portd status") != 0)
		{
			PRINTF("\r\n");
			PRINTF("PORTD= 0x%x\r\n", PORTD);
		}
		else if(strstr(buffer, "adc") != 0)
		{
			PRINTF("\r\n");
			PRINTF("current ADC = %d\r\n", get_adc_value());
		}
		else if(strstr(buffer, "discharge") != 0)
		{
			PRINTF("Starting DIScharge\r\n");
			g_what_to_do = DISCHARGE;
		}
		else if(strstr(buffer, "charge") != 0)
		{
			PRINTF("Starting charge\r\n");
			g_what_to_do = CHARGE;
		}
		else if(strstr(buffer, "stop") != 0)
		{
			PRINTF("stop charging\r\n");
			g_what_to_do = STOP;
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
	else if(length >= 256)
	{
		PRINTF("\r\nERROR ");
		PRINTF("\r\nUART> ");
		length = 0;
	}
}



