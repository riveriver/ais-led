#include "SYSCFG.h"

#define LedGroup1 PA6
#define LedGroup2 PA0
#define LedGroup3 PA1
#define LedGroup4 PA2
#define LedGroup5 PC2

void POWER_INITIAL (void)
{
	OSCCON = 0B01110001;
	INTCON = 0;
	PORTA = 0B00000000;
	TRISA = 0B00000000;
	PORTC = 0B00000000;
	TRISC = 0B00001000;

	WPUA = 0B00000000;
	WPUC = 0B00001000;

	OPTION = 0B00001000;
    PSRCA = 0B11111111;
    PSRCC = 0B11111111;
    PSINKA = 0B11111111;
    PSINKC = 0B11111111;

    MSCON = 0B00110000;

}

void interrupt ISR(void)
{
}

void DelayUs(unsigned char Time)
{
	unsigned char a;
	for(a=0;a<Time;a++)
	{
		NOP();
	}
}

void DelayMs(unsigned char Time)
{
	unsigned char a,b;
	for(a=0;a<Time;a++)
	{
		for(b=0;b<5;b++)
		{
		 	DelayUs(197);
		}
	}
}

void LED_GROUPS_SET(unsigned char State)
{
	LedGroup1 = State;
	LedGroup2 = State;
	LedGroup3 = State;
	LedGroup4 = State;
	LedGroup5 = State;
}

void main(void)
{
	POWER_INITIAL();

	while(1)
	{

		LED_GROUPS_SET(1);
		DelayMs(50);
		LED_GROUPS_SET(0);
		DelayMs(50);

		LED_GROUPS_SET(1);
		DelayMs(50);
		LED_GROUPS_SET(0);
		DelayMs(50);

		LED_GROUPS_SET(1);
		DelayMs(50);
		LED_GROUPS_SET(0);
		DelayMs(50);

		LED_GROUPS_SET(1);
		DelayMs(50);
		LED_GROUPS_SET(0);
		DelayMs(50);

		DelayMs(250);
		DelayMs(250);
	}
}

