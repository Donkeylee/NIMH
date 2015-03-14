#include <stdio.h> 

#include <avr/io.h>

#include <avr/interrupt.h>
#include <avr/signal.h>
#include <string.h>
#include <util/delay.h> 
#include "test.h"


#define SKIP_ROM   0xCC 

#define F_CPU 20000000UL


#define PRINT_FREQ 10
#define ADC_REF_VALUE 1700 //5.1V / 3 = 1.7Volt
#define MAX_CHARGE 1600 //1.6V when full charge
#define MAX_ADC_VALUE 1023 //defined on spec
#define MIN_DISCHARGE 800 //0.8V Minimum discharge


volatile UINT16 g_what_to_do = 0;
extern volatile UINT16 g_elapsed_sec;








int get_adc_value(void)
{
	ADMUX = 0;

	ADCSRA = 0xE7;
	while(ADCSRA &&(1<<ADIF) == 0);

	return ADC;
}

void Init_System(void)
{
	DDRD = 0xFE;
	//PORTD |= 0xC0;

	//보-레이트 115.2kbps,비동기식,패리티 없음,스탑 1bit,문자열 사이즈 8bit,상승 엣지 검출
	//보-레이트 115.2kbps,비동기식,패리티 없음,스탑 1bit,문자열 사이즈 8bit,상승 엣지 검출
	UBRR0H = 0;
	UBRR0L = 10;
	UCSR0A = 0x00;
	UCSR0B = 0x98;             
	UCSR0C = 0x86;

	//Timer init Define
	TCCR1A = 0x00;
	TCCR1B = 0x0C;
	TCCR1C = 0x00;
	//OCR1A = 7811(10Hz)
	OCR1A = 7811;//(100Hz)
	TCNT1 = 0x00;
	TIMSK1 = 0x02;
	TIFR1 = 0x00;
	//CTC mode : 주파수 10Hz(0.1s의 주파수는 10Hz이나 0.1s에는 두번의 Event가 발생하므로 10Hz/2 해서 5Hz 

	
	
	sei(); //Global interrupt enable
}

UINT16 adc_to_volt_converter(UINT32 adc_value)
{
	//return current ADC value to mV value.
	//this function required to fix 
	//ADC_REF_VALUE : This value is current AREF voltage(recomending actual measured value)
	//MAX_ADC_VALUE : this will be current CPU's maximum value of ADC(
	return (UINT16)(adc_value * ADC_REF_VALUE / MAX_ADC_VALUE);
}

void charge_module()
{

	//Charge requires
	//1. charge port on*
	//2. keep monitor current voltage.... todo
	//3. cut off if current voltage is too high(have to decide but... how about... 1.6V??)*
	//4. timer(current time/target time)*
	//5. manual stop*
	//6. current voltage(means voltage before start charge)*
	//7. 10sec average todo
	UINT16 current_voltage;
	UINT16 target_sec = 43200;//12 hrs with 24ohm 5volt supply
	UINT16 last_elapsed_sec;

	current_voltage = adc_to_volt_converter((UINT32)get_adc_value());

	PRINTF("Start charge\r\n");
	PRINTF("Current_voltage ==> %d\r\n", current_voltage);
	PRINTF("Resetting timer to 0\r\n");	
	g_elapsed_sec = 0;
	PRINTF("Target time is %u second(%uhrs or %umins)\r\n", target_sec, target_sec/60/60, target_sec/60);
	PRINTF("Turning on charge module\r\n");
	//PORT xx |= TRUE;

	while(1)
	{

		if((g_elapsed_sec != 0) && (last_elapsed_sec !=  g_elapsed_sec) && (g_elapsed_sec % PRINT_FREQ == 0))
		{
			last_elapsed_sec = g_elapsed_sec;
			current_voltage = adc_to_volt_converter((UINT32)get_adc_value());
			PRINTF("CHARGING[%05u(%03u))/%05u(%03u)] = %04u\r\n", g_elapsed_sec, g_elapsed_sec/60, target_sec, target_sec/60, current_voltage);
		}

		if(g_elapsed_sec >= target_sec)
		{
			PRINTF("Timer end\r\n");
			break;
		}
		if(current_voltage >= MAX_CHARGE)
		{
			PRINTF("Charge voltage reached maximum value!\r\n");
			//break;
		}
		if(g_what_to_do == STOP)
		{
			PRINTF("Stop by user\r\n");
			break;
		}
	}
	PRINTF("Finish charge\r\n");
	PRINTF("Turning off charge module\r\n");
	//PORT xx |= FALSE;
	current_voltage = adc_to_volt_converter((UINT32)get_adc_value());
	PRINTF("Current_voltage ==> %d\r\n\n", current_voltage);
	
}

void discharge_module()
{
	//Discharge requires
	//1. Keep monitor current voltage todo
	//2. cut off if current voltage is too low(may be 0.9V??) *
	//3. measure vaule per second, then printf per 10 second average *
	//4. manual stop *
	//5. discharge port on *

	UINT16 current_voltage;
	UINT16 last_elapsed_sec;
	current_voltage = adc_to_volt_converter((UINT32)get_adc_value());
	PRINTF("discharge\r\n");
	PRINTF("Current_voltage ==> %d\r\n", current_voltage);
	PRINTF("Resetting timer to 0\r\n");	
	g_elapsed_sec = 0;
	PRINTF("Turning on discharge module\r\n");
	//PORT xx |= TRUE;

	while(1)
	{
		if((g_elapsed_sec != 0) && (last_elapsed_sec !=  g_elapsed_sec) && (g_elapsed_sec % PRINT_FREQ == 0))
		{
			last_elapsed_sec = g_elapsed_sec;
			current_voltage = adc_to_volt_converter((UINT32)get_adc_value());
			PRINTF("DISCHARGING[%05u(%03u] = %05u\r\n", g_elapsed_sec, g_elapsed_sec/60, current_voltage);
		}

		if(current_voltage <= MIN_DISCHARGE)
		{
			PRINTF("Discharge voltage reached minimum value!\r\n");
			break;
		}
		if(g_what_to_do == STOP)
		{
			PRINTF("Stop by user\r\n");
			break;
		}
	}
	PRINTF("Finish discharge\r\n");
	PRINTF("Turning off discharge module\r\n");
	//PORT xx |= FALSE;
	current_voltage = adc_to_volt_converter((UINT32)get_adc_value());
	PRINTF("Current_voltage ==> %d\r\n\n", current_voltage);
	
}


void repeat_module()
{
	//Repeat requires
	//1. Current count
	//2. delay for stable??
	//3. Need to start discharge module first

	

	UINT16 current_voltage;
	UINT16 counter;
	current_voltage = adc_to_volt_converter((UINT32)get_adc_value());
	PRINTF("------------------Starting repeat test sequence----------------------\r\n");
	PRINTF("Current_voltage ==> %d\r\n", current_voltage);
	PRINTF("Resetting counter to 0\r\n");	
	counter = 0;	
	



	while(1)
	{
		PRINTF("------------------%u----------------------\r\n", counter++);	
		charge_module();

		if(g_what_to_do == STOP)
		{
			PRINTF("Stop by user\r\n");
			break;
		}
		
		discharge_module();

		if(g_what_to_do == STOP)
		{
			PRINTF("Stop by user\r\n");
			break;
		}
	}
	PRINTF("------------------Finishing repeat test sequence----------------------\r\n");
	PRINTF("Turning off discharge module\r\n");
	current_voltage = adc_to_volt_converter((UINT32)get_adc_value());
	PRINTF("Current_voltage ==> %d\r\n\n", current_voltage);
	
}


int main(void)
{	

	Init_System();
	g_what_to_do = STOP;

	get_adc_value();// Required this.... i don't know why first time read ADC always 0???
	
	
	while(1)
	{
		while(g_what_to_do == STOP);
		

		switch(g_what_to_do)
		{
			case CHARGE :
				charge_module();				
			break;

			case DISCHARGE :
				discharge_module();
			break;
			case REPEAT :
				repeat_module();
			break;
		}
		g_what_to_do = STOP;
	}
}

